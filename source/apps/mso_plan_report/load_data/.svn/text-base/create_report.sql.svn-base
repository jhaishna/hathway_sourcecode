TRUNCATE TABLE MSO_BB_PLAN_REPORT_T;

INSERT INTO MSO_BB_PLAN_REPORT_T
select
t1.PLAN_ID as "PACKAGE CODE",
NULL as "PLAN CODE",
t1.plan_NAME as "PKG NAME",
NULL as "Revised Name",
case 
when t1.BILL_WHEN=0 then
  decode(mysplit(mysplit(t1.event_type,'/',6 ),'_',3), 
         'weekly', '1/7',
         'monthly', 1,
         'bimonthly',2,
         'quarterly',3,
         'semiannual', 6,
         'annual', 12,
         0
        )
else
  t1.BILL_WHEN
end as "PAY TERM",
--t1.BILL_WHEN as "PAY TERM",
t2.ul_speed US,
t2.dl_speed DS,
decode(t1.CITY_NAME,NULL,'*',t1.CITY_NAME) as "CITY REG",
t1.FIXED_AMOUNT+t1.SCALED_AMOUNT as "PRICE",
case
when t2.type!=2 then 
  t1.GRANT_SCALED_AMOUNT+t1.GRANT_FIXED_AMOUNT
else
  0
end as "FREE UNITS",
decode(t2.type, 0, 'Unlimited', 1, 'Limited', 2, 'Unlimited FUP') as "LTD / UNLTD",
decode(t2.indicator, 0, 'POSTPAID', 1, 'PREPAID') as "PLAN TYPE",
decode (t2.bb_technology, 1, 'DOCSIS2', 2, 'DOCSIS3', 3, 'ETHERNET', 4, 'FIBER', 5,'WIRELESS')
as "TECHNOLOGY",
case
when (to_number(t1.priority)>=0    and to_number(t1.priority)<=999) then 'Hardware products'
when (to_number(t1.priority)>=1000 and to_number(t1.priority)<=1999) then 'SME Products'
when (to_number(t1.priority)>=2000 and to_number(t1.priority)<=2999) then 'RETAIL Products'
when (to_number(t1.priority) >=3000 and to_number(t1.priority)<=3999) then 'CyberCafe Products'
when (to_number(t1.priority)>=4000 and to_number(t1.priority)<=3999) then 'Corporate Products'
end as
"PLAN CATEGORY",
decode(t2.type, 2,(t1.GRANT_SCALED_AMOUNT+t1.GRANT_FIXED_AMOUNT), 0 ) as "FAIR USG LIMIT",
t2.FUP_DL_SPEED as "DOWN STREAM AFTER FUP",
t1.usage_rate as "Currency Charges per MB",
'A' as "STATUS",
'HATH' as "PP CODE",
t1.priority as "Priority",
t1.PLAN_DESCR as "Remark",
t2.SERVICE_CODE as "NEW CODE1 (SERVICE CODE)",
t2.QUOTA_CODE as "NEW CODE3 (QUOTA CODE)"
from
tmp_get_bb_prod_info_sis t1,
mso_config_bb_pt_t t2,
plan_master_list_t t3
where
t1.PROVISIONING_TAG=t2.provisioning_tag
and t3.PLAN_NAME=t1.plan_name;
commit;
