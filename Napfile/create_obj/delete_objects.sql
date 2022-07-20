Delete from MSO_CFG_VAT_MAPPING_T;
Delete from MSO_CFG_STATE_TAX_ZONES_T;
Delete from MSO_CFG_STATE_ET_ZONES_T;
Delete from MSO_CFG_STATE_ET_ZONES_TAXES_T;
Delete from MSO_CFG_STATE_VAT_ZONES_T;
Delete from MSO_CFG_STATE_VT_ZONES_TAXES_T;
Delete from MSO_CFG_REFUND_RULE_T;
Delete from MSO_CFG_REFUND_RULE_STEPS_T;
Delete from MSO_CFG_ET_MAPPING_T;
Delete from MSO_CFG_CITY_TAX_ZONES_T;
Delete from MSO_CFG_CITY_ET_ZONES_T;
Delete from MSO_CFG_CITY_ET_ZONES_TAXES_T;
Delete from MSO_CFG_CATV_PT_CHANNEL_MAP_T;
Delete from MSO_CFG_CATV_CHANNLES_T;
Delete from MSO_CFG_BOUQUET_CITY_MAP_T;
Delete from MSO_CFG_BOUQUET_AREA_MAP_T;

delete from config_collections_scenario_t where scenario_name='MSO_CATV';
delete from config_t where poid_type='/config/collections/scenario';

delete from config_collections_profile_t where profile_name='MSO_CATV';
delete from config_t where poid_type='/config/collections/profile';

delete from config_collections_action_t where action_name in ('LATE_Fee','GRACE_DAY_REMINDER','DOWNGRADE_SERVICE','SERVICE_TERMINATION');
delete from config_t where poid_type like '/config/collections/action/custom%';

Delete from DATA_T where name in('MSO_SEQUENCE_TYPE_ADJUSTMENT', 'MSO_SEQUENCE_TYPE_DEPOSIT', 
'MSO_SEQUENCE_TYPE_RECEIPT', 'MSO_SEQUENCE_TYPE_REFUND', 'MSO_SEQUENCE_TYPE_BUS_USER_ACNT', 
'MSO_SEQUENCE_TYPE_BUS_UNIT_ACNT', 'MSO_SEQUENCE_TYPE_END_CUST_ACNT', 'MSO_SEQUENCE_TYPE_AGREEMENT', 
'MSO_NTF_SEQUENCE', 'MSO_PROV_SEQUENCE');

UPDATE 	DATA_T set HEADER_NUM=100 where name in ('MSO_SEQUENCE_TYPE_ADJUSTMENT', 'MSO_SEQUENCE_TYPE_DEPOSIT', 
'MSO_SEQUENCE_TYPE_RECEIPT', 'MSO_SEQUENCE_TYPE_REFUND', 'MSO_SEQUENCE_TYPE_BUS_USER_ACNT', 
'MSO_SEQUENCE_TYPE_BUS_UNIT_ACNT', 'MSO_SEQUENCE_TYPE_END_CUST_ACNT', 'MSO_SEQUENCE_TYPE_AGREEMENT', 'MSO_NTF_SEQUENCE', 
'MSO_PROV_SEQUENCE');

delete from config_collections_scenario_t where scenario_name='MSO_CATV';
delete from config_t where poid_type='/config/collections/scenario';

delete from config_collections_profile_t where profile_name='MSO_CATV';
delete from config_t where poid_type='/config/collections/profile';

delete from config_collections_action_t where action_name in ('LATE_Fee','GRACE_DAY_REMINDER','DOWNGRADE_SERVICE','SERVICE_TERMINATION');
delete from config_t where poid_type like '/config/collections/action/custom%';

delete from mso_cfg_et_mapping_t;

delete from mso_cfg_vat_mapping_t;

delete from mso_cfg_refund_rule_steps_t;
delete from mso_cfg_refund_rule_t;

commit;
