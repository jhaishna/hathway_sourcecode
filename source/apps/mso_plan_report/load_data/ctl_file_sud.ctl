LOAD DATA
INFILE '../bb_plan_report.csv'
INTO TABLE tmp_bb_prod_info_sud
fields terminated by ','
(plan_name,
plan_descr,
plan_id,
deal_name,
deal_descr,
deal_id,
product_name,
product_descr,
product_id,
event_type,
grant_scaled_amount,
grant_fixed_amount,
grant_unit,
validity_offset_value,
validity_offset_unit,
grant_prorate_first,
grant_prorate_last,
usage_rate,
usage_unit,
fixed_amount,
scaled_amount,
sb_prorate_first,
sb_prorate_last,
provisioning_tag,
priority,
tax_code,
tax_when,
city_name,
bill_when)