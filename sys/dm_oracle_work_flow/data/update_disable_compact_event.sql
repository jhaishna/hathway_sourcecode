Rem
Rem $Header: bus_platform_vob/bus_platform/data/sql/update_disable_compact_event.sql /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:46:55 nishahan Exp $
Rem
Rem update_disable_compact_event.sql
Rem
Rem Copyright (c) 2012, Oracle and/or its affiliates. All rights reserved. 
Rem
Rem    NAME
Rem      update_disable_compact_event.sql - <one-line expansion of the name>
Rem
Rem    DESCRIPTION
Rem      This script would used for disabling compact event feature.
Rem
Rem    NOTES
Rem      <other useful comments, qualifications, etc.>
Rem
Rem    MODIFIED   (MM/DD/YY)
Rem    abgeorg     10/09/12 - Created
Rem

UPDATE dd_objects_t SET residency_type = 0 WHERE name LIKE '/event';
UPDATE dd_objects_t SET residency_type = 0 where name LIKE '/event/%';
