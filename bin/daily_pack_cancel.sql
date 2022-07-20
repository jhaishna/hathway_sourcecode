set time on;

set timing on;

alter session force parallel ddl parallel 30;

/* Formatted on 10-05-2019 20:51:51 (QP5 v5.115.810.9015) */
select to_char(sysdate,'dd-mm-yyyy hh24:mi.ss')  startdt from dual;

SELECT   *
                          FROM   mso_den_can_flag;

DROP TABLE dly_can_pan_india;

CREATE TABLE dly_can_pan_india
PARALLEL
AS
   SELECT                                                       /*+ full(a) */
         poid_id0,
            account_obj_id0,
            service_obj_id0,
            plan_obj_id0,
            PACKAGE_ID,
            deal_obj_id0,
            product_obj_id0,
            created_t created_date,
            PURCHASE_END_T pet,
            status_flags
     FROM   pin.purchased_product_T a
    WHERE   status <> 3
            AND purchase_end_t > 0 and purchase_end_t <=
                  (SELECT   ora2inf (TO_CHAR (SYSDATE, 'dd-mon-yyyy'))
                     FROM   DUAL)
            AND service_obj_type = '/service/catv'
	    AND EXISTS (SELECT  /*+parallel (a 6) */  hw_flag
                          FROM   mso_den_can_flag a
                         WHERE   upper(NVL (hw_flag, 'Y')) = 'Y' or upper(NVL (den_flag, 'Y')) = 'Y' or upper(NVL (dsn_flag, 'Y')) = 'Y' );


DROP INDEX tmp_ind_ser2;

CREATE INDEX tmp_ind_ser2
   ON dly_can_pan_india (service_obj_id0);

DROP TABLE dly_can_pan_fin;

CREATE TABLE dly_can_pan_fin
AS
   SELECT   a.*, (SELECT   PROVISIONING_TAG
                    FROM   pin.product_T b
                   WHERE   b.poid_id0 = a.PRODUCT_OBJ_ID0)
                    prov_tag
     FROM   dly_can_pan_india a;

DROP TABLE dly_can_pan_fin2;

CREATE TABLE dly_can_pan_fin2
AS
   SELECT   b.*, (SELECT   name
                    FROM   pin.plan_T a
                   WHERE   a.poid_id0 = b.plan_obj_id0)
                    plan_name, (SELECT   name
                                  FROM   pin.product_T a
                                 WHERE   a.poid_id0 = b.product_obj_id0)
                                  product_name
     FROM   dly_can_pan_fin b;

DROP TABLE hw_base_pack;

CREATE TABLE hw_base_pack
AS
   SELECT   ACCOUNT_OBJ_ID0,
            deal_obj_id0,
            product_obj_id0,
            prov_tag,
            package_id,
            plan_obj_id0,
            SERVICE_OBJ_ID0
     FROM   dly_can_pan_fin2 a, account_t b
    WHERE   b.poid_id0 = a.account_obj_id0
            AND SUBSTR (b.business_type, -1) IN (0, 1)
	    AND EXISTS (SELECT   hw_flag
                          FROM   mso_den_can_flag
                         WHERE   upper(NVL (hw_flag, 'Y')) = 'Y');

DROP TABLE den_base_pack;

CREATE TABLE den_base_pack
AS
   SELECT   ACCOUNT_OBJ_ID0,
            deal_obj_id0,
            product_obj_id0,
            prov_tag,
            package_id,
            plan_obj_id0,
            SERVICE_OBJ_ID0
     FROM   dly_can_pan_fin2 a, account_t b
    WHERE       b.poid_id0 = a.account_obj_id0
            AND SUBSTR (b.business_type, -1) IN (2, 3)
            AND EXISTS (SELECT   den_flag
                          FROM   mso_den_can_flag
                         WHERE   upper(NVL (den_flag, 'Y')) = 'Y');

DROP TABLE dsn_base_pack;

CREATE TABLE dsn_base_pack
AS
   SELECT   ACCOUNT_OBJ_ID0,
            deal_obj_id0,
            product_obj_id0,
            prov_tag,
            package_id,
            plan_obj_id0,
            SERVICE_OBJ_ID0
     FROM   dly_can_pan_fin2 a, account_t b
    WHERE   b.poid_id0 = a.account_obj_id0
            AND SUBSTR (b.business_type, -1) IN (4, 5)
	    AND EXISTS (SELECT   dsn_flag
                          FROM   mso_den_can_flag
                         WHERE   upper(NVL (dsn_flag, 'Y')) = 'Y');

DROP TABLE  others_base_pack;

CREATE TABLE others_base_pack
AS
   SELECT   ACCOUNT_OBJ_ID0,
            deal_obj_id0,
            product_obj_id0,
            prov_tag,
            package_id,
            plan_obj_id0,
            SERVICE_OBJ_ID0
     FROM   dly_can_pan_fin2
   MINUS
   (SELECT   ACCOUNT_OBJ_ID0,
             deal_obj_id0,
             product_obj_id0,
             prov_tag,
             package_id,
             plan_obj_id0,
             SERVICE_OBJ_ID0
      FROM   hw_base_pack
    UNION
    SELECT   ACCOUNT_OBJ_ID0,
             deal_obj_id0,
             product_obj_id0,
             prov_tag,
             package_id,
             plan_obj_id0,
             SERVICE_OBJ_ID0
      FROM   den_base_pack
    UNION
    SELECT   ACCOUNT_OBJ_ID0,
             deal_obj_id0,
             product_obj_id0,
             prov_tag,
             package_id,
             plan_obj_id0,
             SERVICE_OBJ_ID0
      FROM   dsn_base_pack      );

INSERT INTO pin.cancel_plan_mta_t
   SELECT   1,
            ROHAN.SER_ASSIGN_MTA_SEQ.NEXTVAL,
            '/cancel_plan_mta',
            1,
            ora2inf (TO_CHAR (SYSDATE, 'DD-MON-YYYY')),
            ora2inf (TO_CHAR (SYSDATE, 'DD-MON-YYYY')),
            'S',
            'S',
            1,
            ACCOUNT_OBJ_ID0,
            '/account',
            0,
            1,
            deal_obj_id0,
            '/deal',
            0,
            NULL,
            package_id,
            1,
            plan_obj_id0,
            '/plan',
            0,
            1,
            SERVICE_OBJ_ID0,
            '/service/catv',
            0,
            0,
            1,
            6464305206,
            '/account',
            0,
            TO_CHAR (SYSDATE, 'YYYYMMDD') || '_HW_00_BASEPACK_CAN',
            'BASE_PACK_CANCEL',
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
     FROM   hw_base_pack
    WHERE   PRODUCT_OBJ_ID0 IN  (select distinct poid_id0 from product_t where PROVISIONING_TAG='PROV_NCF_PRO');
                  --('15280684344', '18794124256', '18794125792', '18794750593');

COMMIT;

SELECT   TO_CHAR (SYSDATE, 'dd-mm-yyyy hh24:mi.ss') startdt1 FROM DUAL;

INSERT INTO pin.cancel_plan_mta_t
   SELECT   1,
            ROHAN.SER_ASSIGN_MTA_SEQ.NEXTVAL,
            '/cancel_plan_mta',
            1,
            ora2inf (TO_CHAR (SYSDATE, 'DD-MON-YYYY')),
            ora2inf (TO_CHAR (SYSDATE, 'DD-MON-YYYY')),
            'S',
            'S',
            1,
            ACCOUNT_OBJ_ID0,
            '/account',
            0,
            1,
            deal_obj_id0,
            '/deal',
            0,
            NULL,
            package_id,
            1,
            plan_obj_id0,
            '/plan',
            0,
            1,
            SERVICE_OBJ_ID0,
            '/service/catv',
            0,
            99,
            1,
            6464305206,
            '/account',
            0,
            TO_CHAR (SYSDATE, 'YYYYMMDD') || '_HW_00_OTHERPACK_CAN',
            'OTHER_PACK_CANCEL',
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
     FROM   hw_base_pack
     WHERE  prov_tag <> 'ADDON_HD_PVR';

COMMIT;


INSERT INTO pin.cancel_plan_mta_t
   SELECT   1,
            ROHAN.SER_ASSIGN_MTA_SEQ.NEXTVAL,
            '/cancel_plan_mta',
            1,
            ora2inf (TO_CHAR (SYSDATE, 'DD-MON-YYYY')),
            ora2inf (TO_CHAR (SYSDATE, 'DD-MON-YYYY')),
            'S',
            'S',
            1,
            ACCOUNT_OBJ_ID0,
            '/account',
            0,
            1,
            deal_obj_id0,
            '/deal',
            0,
            NULL,
            package_id,
            1,
            plan_obj_id0,
            '/plan',
            0,
            1,
            SERVICE_OBJ_ID0,
            '/service/catv',
            0,
            0,
            1,
            6464305206,
            '/account',
            0,
            TO_CHAR (SYSDATE, 'YYYYMMDD') || '_DEN_00_BASEPACK_CAN',
            'BASE_PACK_CANCEL',
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
     FROM   den_base_pack
    WHERE   PRODUCT_OBJ_ID0 IN  (select distinct poid_id0 from product_t where PROVISIONING_TAG='PROV_NCF_PRO');
                  --('15280684344', '18794124256', '18794125792', '18794750593');

COMMIT;

SELECT   TO_CHAR (SYSDATE, 'dd-mm-yyyy hh24:mi.ss') startdt1 FROM DUAL;

INSERT INTO pin.cancel_plan_mta_t
   SELECT   1,
            ROHAN.SER_ASSIGN_MTA_SEQ.NEXTVAL,
            '/cancel_plan_mta',
            1,
            ora2inf (TO_CHAR (SYSDATE, 'DD-MON-YYYY')),
            ora2inf (TO_CHAR (SYSDATE, 'DD-MON-YYYY')),
            'S',
            'S',
            1,
            ACCOUNT_OBJ_ID0,
            '/account',
            0,
            1,
            deal_obj_id0,
            '/deal',
            0,
            NULL,
            package_id,
            1,
            plan_obj_id0,
            '/plan',
            0,
            1,
            SERVICE_OBJ_ID0,
            '/service/catv',
            0,
            99,
            1,
            6464305206,
            '/account',
            0,
            TO_CHAR (SYSDATE, 'YYYYMMDD') || '_DEN_00_OTHERPACK_CAN',
            'OTHER_PACK_CANCEL',
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
     FROM   den_base_pack
     WHERE  prov_tag <> 'ADDON_HD_PVR';

COMMIT;

INSERT INTO pin.cancel_plan_mta_t
   SELECT   1,
            ROHAN.SER_ASSIGN_MTA_SEQ.NEXTVAL,
            '/cancel_plan_mta',
            1,
            ora2inf (TO_CHAR (SYSDATE, 'DD-MON-YYYY')),
            ora2inf (TO_CHAR (SYSDATE, 'DD-MON-YYYY')),
            'S',
            'S',
            1,
            ACCOUNT_OBJ_ID0,
            '/account',
            0,
            1,
            deal_obj_id0,
            '/deal',
            0,
            NULL,
            package_id,
            1,
            plan_obj_id0,
            '/plan',
            0,
            1,
            SERVICE_OBJ_ID0,
            '/service/catv',
            0,
            0,
            1,
            6464305206,
            '/account',
            0,
            TO_CHAR (SYSDATE, 'YYYYMMDD') || '_DSN_00_BASEPACK_CAN',
            'BASE_PACK_CANCEL',
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
     FROM   dsn_base_pack
    WHERE   PRODUCT_OBJ_ID0 IN  (select distinct poid_id0 from product_t where PROVISIONING_TAG='PROV_NCF_PRO');
                  --('15280684344', '18794124256', '18794125792', '18794750593');

COMMIT;

SELECT   TO_CHAR (SYSDATE, 'dd-mm-yyyy hh24:mi.ss') startdt1 FROM DUAL;

INSERT INTO pin.cancel_plan_mta_t
   SELECT   1,
            ROHAN.SER_ASSIGN_MTA_SEQ.NEXTVAL,
            '/cancel_plan_mta',
            1,
            ora2inf (TO_CHAR (SYSDATE, 'DD-MON-YYYY')),
            ora2inf (TO_CHAR (SYSDATE, 'DD-MON-YYYY')),
            'S',
            'S',
            1,
            ACCOUNT_OBJ_ID0,
            '/account',
            0,
            1,
            deal_obj_id0,
            '/deal',
            0,
            NULL,
            package_id,
            1,
            plan_obj_id0,
            '/plan',
            0,
            1,
            SERVICE_OBJ_ID0,
            '/service/catv',
            0,
            99,
            1,
            6464305206,
            '/account',
            0,
            TO_CHAR (SYSDATE, 'YYYYMMDD') || '_DSN_00_OTHERPACK_CAN',
            'OTHER_PACK_CANCEL',
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
     FROM   dsn_base_pack
     WHERE  prov_tag <> 'ADDON_HD_PVR';

COMMIT;


SELECT   TO_CHAR (SYSDATE, 'dd-mm-yyyy hh24:mi.ss') enddt FROM DUAL;

--drop table DLY_CAN_PAN_FIN2;
--drop table DLY_CAN_PAN_FIN;
--drop table DLY_CAN_PAN_INDIA;




exit;
