CREATE OR REPLACE VIEW job_history_vw
("POID_DB", 
 "POID_TYPE", 
 "POID_ID0", 
 "POID_REV", 
 "TRACKING_ID", 
 "PROCESS_START_T", 
 "PROCESS_END_T", 
 "PROCESSING_TIME",
 "JOB_STATUS", 
 "STATUS", 
 "REASON_ID",
 "REASON_DOMAIN_ID",
 "TEMPLATE_OBJ_DB", 
 "TEMPLATE_OBJ_TYPE", 
 "TEMPLATE_OBJ_ID0", 
 "TEMPLATE_OBJ_REV", 
 "USER_TAG", 
 "TEMPLATE_STATUS",
 "TEMPLATE_NAME",
 "CAT_TYPE",
 "JOB_TYPE",
 "CREATED_BY",
 "PARENT_TEMPLATE_OBJ_DB", 
 "PARENT_TEMPLATE_OBJ_TYPE", 
 "PARENT_TEMPLATE_OBJ_ID0", 
 "PARENT_TEMPLATE_OBJ_REV", 
 "JOB_OBJ_DB", 
 "JOB_OBJ_TYPE", 
 "JOB_OBJ_ID0", 
 "JOB_OBJ_REV",
 "WORKFLOW_OBJ_DB",
 "WORKFLOW_OBJ_TYPE",
 "WORKFLOW_OBJ_ID0",
 "WORKFLOW_OBJ_REV"
 ) AS 
  ((select 
  a.poid_db ,
  a.poid_type,
  a.poid_id0,
  a.poid_rev,
  a.tracking_id,
  a.process_start_t,
  a.process_end_t,
  a.processing_time,
  decode(b.status, 11,6,12,7,13,8,b.status),
  b.status,
  b.reason_id,
  b.reason_domain_id,
  c.poid_db,
  c.poid_type,
  c.poid_id0,
  c.poid_rev,
  c.user_tag,
  c.status,
  c.name,
  c.cat_type,
  c.job_type,
  c.created_by,
  c.parent_template_obj_db,
  c.parent_template_obj_type,
  c.parent_template_obj_id0,
  c.parent_template_obj_rev,
  c.job_obj_db, 
  c.job_obj_type,
  c.job_obj_id0,
  c.job_obj_rev,
  b.workflow_obj_db, 
  b.workflow_obj_type,
  b.workflow_obj_id0,
  b.workflow_obj_rev
  from job_t a, 
      job_boc_t b,
      job_template_t c
  where a.poid_id0 = b.obj_id0
  and b.template_obj_id0 = c.poid_id0
  and (b.status >= 5 or (b.status=1 and c.cat_type=612) ))
  UNION ALL
  (select 
  a.job_obj_db ,
  a.job_obj_type,
  a.job_obj_id0,
  a.job_obj_rev,
  to_char(a.job_obj_id0),
  a.process_start,
  a.process_end,
  (a.process_end - a.process_start),
  decode(a.failrecords, 0,5,6) ,
  decode(a.failrecords, 0,5,6),
  0,
  0,
  a.poid_db,
  decode(process_name,'pin_export_price','/job_template/price_sync',
                      'fm_price','/job_template/price_sync',
                      'pin_bill_accts', '/job_template/billling',
                      'Bill-Now','/job_template/billling',
                      'Bill-ON-DEMAND','/job_template/billling',
                      'Auto-Trigger','/job_template/billling',
                      'pin_trial_bill_accts','/job_template/billling',
                      'pin_collect', '/job_template/collect',
                      'pin_inv_accts', '/job_template/invoice',
                      'pin_ledger_report', '/job_template/ledger_report',
                      '/job_template'),
  -1,
   0,
   NULL,
   10100,
   'System Job',
   decode(process_name,'pin_export_price',605,
                      'fm_price',605,
                      'pin_bill_accts', 601,
                      'Bill-Now',601,
                      'Bill-ON-DEMAND',601,
                      'Auto-Trigger',601,
                      'pin_trial_bill_accts',601,
                      'pin_collect', 602,
                      'pin_inv_accts', 603,
                      'pin_ledger_report', 604,
                      600),
  1,
  'System',
  a.poid_db,
  decode(process_name,'pin_export_price','/job_template/price_sync',
                      'fm_price','/job_template/price_sync',
                      'pin_bill_accts', '/job_template/billling',
                      'Bill-Now','/job_template/billling',
                      'Bill-ON-DEMAND','/job_template/billling',
                      'Auto-Trigger','/job_template/billling',
                      'pin_trial_bill_accts','/job_template/billling',
                      'pin_collect', '/job_template/collect',
                      'pin_inv_accts', '/job_template/invoice',
                      'pin_ledger_report', '/job_template/ledger_report',
                      '/job_template'),
  -1,
   0,
   a.poid_db,
  '/job/boc',
  -1,
   0,
   a.poid_db,
  '/job/boc',
  -1,
   0
  from proc_aud_t a
  where a.job_obj_type like '%system%')
);
/
show errors;
/
CREATE OR REPLACE VIEW rev_assurance_vw
 ("POID_ID0", 
  "FAILRECORDS", 
  "SUCCESSRECORDS", 
  "INPUTRECORDS", 
  "NUMBER_OF_BILLS", 
  "NUM_DEALS", 
  "NUM_DISCOUNTS", 
  "NUM_PRODUCTS", 
  "NUM_PLAN_LISTS", 
  "NUM_PLANS", 
  "NUM_SPONSORSHIPS") AS 
  ((select 
   a.job_obj_id0,
   sum(a.failrecords),
   sum(a.successrecords),
   sum(a.inputrecords),
   sum(b.number_of_bills),
   0,
   0, 
   0, 
   0, 
   0,
   0  
   from proc_aud_t a, proc_aud_bill_t b 
   where a.poid_id0 = b.obj_id0
   and a.job_obj_id0 != 0
   group by a.job_obj_id0)
   union All
   (select 
   a.job_obj_id0,
   sum(a.failrecords),
   sum(a.successrecords),
   sum(a.inputrecords),
   0, 
   0, 
   0,
   0, 
   0, 
   0,
   0  
   from proc_aud_t a
   where a.process_name = 'pin_ledger_report'
   and a.job_obj_id0 != 0
   group by a.job_obj_id0)
   union All
   (select 
   a.job_obj_id0,
   a.failrecords,
   a.successrecords,
   a.inputrecords,
   0,
   b.num_deals,
   b.num_discounts,
   b.num_products,
   b.num_plan_lists,
   b.num_plans, 
   b.num_sponsorships
   from proc_aud_t a, proc_aud_price_sync_t b
   where a.poid_id0 = b.obj_id0
   and a.job_obj_id0 != 0)
   union All
   (select 
   a.obj_id0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0, 
   0
   from job_boc_t a
   where a.status >= 5
   and NOT EXISTS (SELECT * FROM proc_aud_t b
                    WHERE a.obj_id0 = b.job_obj_id0))
   UNION ALL
   (select
   a.obj_id0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0
   from job_boc_t a
   where a.status =1 and a.template_obj_type='/job_template/workflow'
   )
);
/
show errors;
/
create or replace PACKAGE job_history_pkg wrapped 
a000000
367
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
9
338 179
4QPsgRI2SEU8gRxFLb07/m9bl68wg9fxAF5qfHRAk/pkUOe/WdhYjoAuLv3aSdm4WM+xAmRr
dkuluy2QGzaD9dQXmjEjLJOnXcs9Of2snHPRegCqz7manRUFHgtrTPsvF6Mlvza/YaP2SacS
QzGTOc2p3kRqJuRrtf6r1EsT/7dXBfV+lD1BvavxLwMoB9KE8KyIDIbyE1bUCS+3zDaD3QDC
o++aGV+OdhbWYq9ma2nm4oHTjzaVmBT9mivbnaK749Zwx4MqCq3kBobUUEm5hyk7orXlD1AD
1Vbjpx7HvH4sUSpD1MAcqF5yN9lrZnZt6LqSCqLOhSKH3hC5fXdp03e6nEeAmuRFVgm0QrWY
Ll6Hzx23LjsD

/
show errors;
/
create or replace PACKAGE BODY job_history_pkg wrapped 
a000000
367
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
b
18b7 8b6
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
hmy930ewgSUdUc0snndlkQhdlcFOCJa1+4ANxME=

/
show errors;
/
create or replace PACKAGE failed_transactions_pkg wrapped 
a000000
367
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
9
259 15c
G7QPmzCNgvnA0xcmQnk/C2GePO4wg433rvZqfHSVv/+wUFLWpNTirT5h7Fgjn8iNX0GitCSX
YHg8P6qLGmxf4j4Tr3DZnbAX0qshDNi9h5KB+E63ubyS12GqaDsk37pzzxJiccWmyiCTKjZ+
wwSECevAGHTR++vhM8RlXrKAhGEBrOyiH316MdbbUY9Q9p3RsIyZW3D1uPvmEg/zD7U5ejJd
F+PkjqITfdiDJ8CIj1KQ6l//gBjLMWiomv/ebu06sJle/h8C0DlzV+O3D+N+9A2j10c3hoTi
J9ORrdBpS7CptBjcf7rxdjirGMYxD42VUO4X2xHblXQh2UU0zjm9WvcX

/
show errors;
create or replace PACKAGE BODY failed_transactions_pkg wrapped 
a000000
367
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
abcd
b
261f 9b6
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
KA==

/
show errors;
