Rem
Rem $Header: bus_platform_vob/bus_platform/data/sql/update_compact_event.sql /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:46:50 nishahan Exp $
Rem
Rem update_compact_event.sql
Rem
Rem Copyright (c) 2010, 2012, Oracle and/or its affiliates. 
Rem All rights reserved. 
Rem
Rem    NAME
Rem      update_compact_event.sql - Update for compact event feature for Oracle
Rem
Rem    DESCRIPTION
Rem      This file updates the residency types of certain storable class and
Rem	 adds compact_sub_event column to event_t.
Rem      This is done enable compact event feature for Oracle.
Rem
Rem    NOTES
Rem      <other useful comments, qualifications, etc.>
Rem
Rem    MODIFIED   (MM/DD/YY)
Rem    meiyang     12/15/12 - Created
Rem    meiyang     06/06/12 - Updated to list the usage events to compact
Rem    shradhak    06/14/12 - Delayed events to be considered as compact
Rem    abgeorg     07/25/12 - Updated the residency type of /event to 1. This enables working of hybrid search.

Rem  -- usage events to be compact

update dd_objects_t set residency_type=1 where name = '/event';
update dd_objects_t set residency_type=301 where name = '/event/activity/content';
update dd_objects_t set residency_type=301 where name = '/event/activity/telco';
update dd_objects_t set residency_type=301 where name = '/event/session/dialup';
update dd_objects_t set residency_type=301 where name = '/event/session/telco';
update dd_objects_t set residency_type=301 where name = '/event/session/telco/gsm';
update dd_objects_t set residency_type=301 where name = '/event/session/telco/gprs';
update dd_objects_t set residency_type=301 where name = '/event/session/telco/gprs/master';
update dd_objects_t set residency_type=301 where name = '/event/session/telco/gprs/subsession';
update dd_objects_t set residency_type=301 where name = '/event/session/telco/imt';
update dd_objects_t set residency_type=301 where name = '/event/session/telco/pdc';

Rem  -- all delayed events to be compact

update dd_objects_t set residency_type=301 where name LIKE '/event/delayed%';

Rem  --  session specific tmp table is used to load profile data
Rem  --  so this table is not loaded and it does not have subtable data
update dd_objects_t set residency_type=304 where name = '/event/delayed/tmp_profile_event_ordering';

Rem  -- create compact column
DECLARE
   col_exist integer;
BEGIN
   -- Check if the column COMPACT_SUB_EVENT exists.
   SELECT count(*) into col_exist FROM user_tab_columns WHERE table_name = 'EVENT_T' AND column_name = 'COMPACT_SUB_EVENT';

   IF col_exist = 0 THEN
      BEGIN
          EXECUTE IMMEDIATE 'ALTER TABLE event_t ADD (COMPACT_SUB_EVENT CLOB)';
      END;
   END IF;
END; 
/

commit;
quit;
