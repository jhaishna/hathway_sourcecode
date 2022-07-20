/*******************************************************************
 *
 * Copyright (c) 1999, 2009, Hathway and/or its affiliates.
 * All rights reserved.
 *
 *      This material is the confidential property of Hathway Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Hathway license or sublicense agreement.
 *
 *******************************************************************/
/*******************************************************************
 *
 * Copyright (c) 1999, 2009, Hathway and/or its affiliates.
 * All rights reserved.
 *
 *      This material is the confidential property of Hathway Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Hathway license or sublicense agreement.
 *
 *******************************************************************/

/***************************************************************************************************************
*VERSION |   DATE    |    Author        |               DESCRIPTION                                            *
*--------------------------------------------------------------------------------------------------------------*
* 0.1    |09/03/2017 | Tony             | Functions to search, get and set args for notification to business users
****************************************************************************************************************/


/*******************************************************************
 * Forward declarations
 ******************************************************************/

static void
fm_mso_ntf_common_get_usage_type_descr(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        int32                   *flags,
        char                    **descr,
        pin_errbuf_t            *ebufp);


static int
fm_mso_ntf_common_check_delivery_status(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        char                    *usage_type,
        char                    *channel,
        pin_errbuf_t            *ebufp);


static void
fm_mso_ntf_common_get_business_users(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **out_flistpp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Function definitions
 ******************************************************************/


/*******************************************************************
 * Function : fm_mso_ntf_common_get_usage_type_descr
 * Descr    : To fetch the mapping between action flag int value
 *            and its corresponding usage type description.
 *            The configuration is stored in /mso_ntf_cfg_usg_type_map
 ******************************************************************/
static void
fm_mso_ntf_common_get_usage_type_descr(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        int32                   *flags,
        char                    **descr,
        pin_errbuf_t            *ebufp)
{

        pin_flist_t     *srch_flistp = NULL,
                        *args_flistp = NULL,
                        *res_flistp  = NULL,
                        *r_flistp;

        poid_t          *srch_poidp = NULL,
                        *pdp        = NULL;

        char            *usage_type = NULL,
                         srch_template[255] = "select x from /mso_ntf_cfg_usg_type_map where F1 = V1 \0";

        int64            database;

        int32            search_flag = SRCH_DISTINCT;

        /****************************************
         * Search Template
         ****************************************
        0 PIN_FLD_POID      POID [0] 0.0.0.1 /search -1 0
        0 PIN_FLD_FLAGS      INT [0] 256
        0 PIN_FLD_TEMPLATE   STR [0] "select x from /mso_ntf_cfg_usg_type_map where F1 = V1 "
        0 PIN_FLD_ARGS     ARRAY [1] allocated 2, used 0
        1    PIN_FLD_FLAGS   INT [0] 1
        0 PIN_FLD_RESULTS  ARRAY [0] allocated 2, used 0
        *****************************************/

        if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

        PIN_ERRBUF_CLEAR(ebufp);

        pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
        database = PIN_POID_GET_DB(pdp);

        srch_poidp = PIN_POID_CREATE(database, "/search", -1, ebufp);

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, (void *)srch_poidp, ebufp);

        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &search_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, &srch_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_FLAGS, flags, ebufp);

        PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ntf_common_get_usage_type_descr search input flist", srch_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &res_flistp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ntf_common_get_usage_type_descr search output flist", res_flistp);

        r_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        if(r_flistp == NULL){
           PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Unable to map usage flag to usage type description");
           return;
        }

        *descr = (char *) PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_USAGE_TYPE, 0, ebufp);
        if(PIN_ERRBUF_IS_ERR(ebufp)){

            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Unable to get usage type description");
        }

        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
}


/*******************************************************************
 * Function : fm_mso_ntf_common_check_delivery_status
 * Descr    : To check if the day's first email or sms of each usage
 *            type has been sent to business users for verification.
 *            The current date is stored in /mso_ntf_bus_delivery_status
 *            0 - already sent
 *            1 - not sent
 ******************************************************************/
static int
fm_mso_ntf_common_check_delivery_status(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        char                    *usage_type,
        char                    *channel,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t     *srch_flistp = NULL,
                        *args_flistp = NULL,
                        *res_flistp  = NULL,
                        *write_flistp = NULL,
                        *r_flistp;

        poid_t          *srch_poidp = NULL,
                        *pdp        = NULL;

        char            srch_template[255] = "select X from /mso_ntf_bus_delivery_status where F1 = V1 and F2 = V2 and F3 = V3 \0",
                        srch_poid_template[255] = "select X from /mso_ntf_bus_delivery_status where F1 = V1 and F2 = V2 \0",
                        *msg = NULL;

        int64           database;

        int32           search_flag = SRCH_DISTINCT;

        struct tm       *tm = NULL;
        time_t          round_for_date = 0,
                        curr_time = 0;

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Enter function fm_mso_ntf_common_check_delivery_status");
        /**
         * Get current time
         */
        pin_virtual_time(&curr_time);
        tm =  localtime(&curr_time);

        //Round timestamp to midnight
        tm->tm_sec = 0;
        tm->tm_min = 0;
        tm->tm_hour = 0;

        round_for_date = mktime(tm);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Current time set");

        /****************************************
         * Search Template
         ****************************************
        0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
        0 PIN_FLD_FLAGS           INT [0] 256
        0 PIN_FLD_TEMPLATE        STR [0] "select X from /mso_ntf_bus_delivery_status where F1 = V1 and F2 = V2 and F3 = V3 "
        0 PIN_FLD_ARGS          ARRAY [1] allocated 2, used 0
        1    PIN_FLD_USAGE_TYPE    STR [0] "NTF_SERVICE_ACTIVATION"
        0 PIN_FLD_ARGS          ARRAY [2] allocated 2, used 0
        1    PIN_FLD_CHANNEL       STR [0] "Email"
        0 PIN_FLD_ARGS          ARRAY [3] allocated 2, used 0
        1    PIN_FLD_TSTAMP_VAL TSTAMP [0] (0)
        0 PIN_FLD_RESULTS       ARRAY [0] allocated 2, used 0
        *****************************************/

        if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

        PIN_ERRBUF_CLEAR(ebufp);

        msg = (char *) malloc(sizeof(char) * 255);
        if(msg == NULL){
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Unable to allocate memory for logging");
            pin_set_err(ebufp, PIN_ERRLOC_FLIST, PIN_ERRCLASS_SYSTEM_RETRYABLE, PIN_ERR_NO_MEM, 0, 0, 0);
            return;
        }
        memset(msg, '\0', sizeof(msg));

        sprintf(msg, "Current time is %d/%d/%d %d:%d:%d", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

        pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
        database = PIN_POID_GET_DB(pdp);

        srch_poidp = PIN_POID_CREATE(database, "/search", -1, ebufp);

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, (void *)srch_poidp, ebufp);

        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &search_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, &srch_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_USAGE_TYPE, usage_type, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CHANNEL, channel, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TSTAMP_VAL, &round_for_date, ebufp);

        PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ntf_common_check_delivery_status input flist", srch_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &res_flistp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ntf_common_check_delivery_status output flist", res_flistp);

        r_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

        if(r_flistp == NULL){

            sprintf(msg, "1st %s notification for usage type %s not sent yet to business", usage_type, channel);
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

            //Update /mso_ntf_bus_delivery_status for the current date

            //Get POID of corresponding usage type and channel
            PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, &srch_poid_template, ebufp);

            PIN_FLIST_ELEM_DROP(srch_flistp, PIN_FLD_ARGS, 3, ebufp);

            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ntf_common_check_delivery_status POID input flist", srch_flistp);

            PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &res_flistp, ebufp);

            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ntf_common_check_delivery_status POID output flist", res_flistp);

            r_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

            if(r_flistp == NULL){

                memset(msg, '\0', sizeof(msg));
                sprintf(msg, "Missing configuration entry in /mso_ntf_bus_delivery_status for usage type %s and channel %s", usage_type, channel);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, msg);

                goto cleanup;
            }
            else{

                pdp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_POID, 0, ebufp);
                if(PIN_POID_IS_NULL(pdp)){
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_mso_ntf_common_check_delivery_status POID value not found");
                    goto cleanup;
                }

                write_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_POID, pdp, ebufp);
                PIN_FLIST_FLD_SET(write_flistp, PIN_FLD_TSTAMP_VAL, &round_for_date, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ntf_common_check_delivery_status write input flist", write_flistp);

                PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flistp, &res_flistp, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ntf_common_check_delivery_status write output flist", res_flistp);

                PIN_FLIST_DESTROY_EX(&write_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Enter function fm_mso_ntf_common_check_delivery_status");
                return 1;
           }

        }
        else{

            goto cleanup;
        }

cleanup:
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        free(msg);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Exit function fm_mso_ntf_common_check_delivery_status");
        return 0;
}


/*******************************************************************
 * Function : fm_mso_ntf_common_get_business_users
 * Descr    : To get the list of active business users to whom
 *            a copy of the notification has to be sent
 ******************************************************************/
static void
fm_mso_ntf_common_get_business_users(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **out_flistpp,
        pin_errbuf_t            *ebufp)
{

        pin_flist_t     *srch_flistp = NULL,
                        *args_flistp = NULL,
                        *res_flistp  = NULL,
                        *r_flistp;

        poid_t          *srch_poidp = NULL,
                        *pdp        = NULL;

        char            srch_template[255] = "select X from /mso_ntf_business_users where F1 = V1 \0",
                        *msg = NULL;

        int64           database;

        int32           search_flag = SRCH_DISTINCT,
                        status = PIN_STATUS_ACTIVE;


        /****************************************
         * Search Template
         ****************************************
        0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
        0 PIN_FLD_FLAGS           INT [0] 256
        0 PIN_FLD_TEMPLATE        STR [0] "select X from /mso_ntf_business_users where F1 = V1 "
        0 PIN_FLD_ARGS          ARRAY [1] allocated 2, used 0
        1    PIN_FLD_STATUS      ENUM [0] 10100
        0 PIN_FLD_RESULTS       ARRAY [0] allocated 2, used 0
        *****************************************/

        if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

        PIN_ERRBUF_CLEAR(ebufp);

        msg = (char *) malloc(sizeof(char) * 255);
        if(msg == NULL){
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Unable to allocate memory for logging");
            pin_set_err(ebufp, PIN_ERRLOC_FLIST, PIN_ERRCLASS_SYSTEM_RETRYABLE, PIN_ERR_NO_MEM, 0, 0, 0);
            return;
        }
        memset(msg, '\0', sizeof(msg));

        pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
        database = PIN_POID_GET_DB(pdp);

        srch_poidp = PIN_POID_CREATE(database, "/search", -1, ebufp);

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, (void *)srch_poidp, ebufp);

        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &search_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, &srch_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);

        PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ntf_common_get_business_users input flist", srch_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &res_flistp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ntf_common_get_business_users output flist", res_flistp);

        if(res_flistp){

            *out_flistpp = PIN_FLIST_COPY(res_flistp, ebufp);
        }

        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        free(msg);
}
