<?xml version="1.0" encoding="UTF-8"?>
<!--
  **************************************************************************
* Copyright (c) 2005, 2013, Oracle and/or its affiliates. All rights reserved. 
  *
  * This material is the confidential property of Oracle Corporation
  * or its licensors and may be used, reproduced, stored or transmitted
  * only in accordance with a valid Oracle license or sublicense agreement.
  **************************************************************************
-->
<xsl:stylesheet version="1.0" xmlns="http://www.portal.com/schemas/BusinessConfig" xmlns:bc="http://www.portal.com/schemas/BusinessConfig" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" exclude-result-prefixes="bc">
	
	<xsl:output method="xml" indent="yes"/>
	
	<xsl:template match="/">
		<BusinessConfiguration xmlns="http://www.portal.com/schemas/BusinessConfig" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.portal.com/schemas/BusinessConfig business_configuration.xsd">
			<BusParamConfiguration>
			<BusParamConfigurationList>
			<ParamClass desc="Business logic parameters for Billing" name="billing">										
				<xsl:apply-templates select="/bc:BusinessConfiguration/bc:BusParamConfigurationClass/bc:BusParamsBilling/bc:*"/>
			</ParamClass>
			</BusParamConfigurationList>
			</BusParamConfiguration>
		</BusinessConfiguration>
	</xsl:template>
	
	<xsl:template match="bc:BillingCycleOffset">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>billing_cycle_offset</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
			The offset from the start of the day for the billing cycle.  This value is in hours and the default is 0 (12 AM). Max 23.
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:variable name="value" select="normalize-space(text())"/>
			<xsl:element name="Value">
				<xsl:value-of select="$value"/>
			</xsl:element>
		</xsl:element>
	</xsl:template>
	
	<xsl:template match="bc:MoveDayForward">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>move_day_forward</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
			The direction to apply billing when a BDOM is not contained in the month.  The default is to use the first day of the next month.
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:element name="Value">
				<xsl:choose>
					<xsl:when test="normalize-space(text()) = &apos;firstDay&apos;">
						<xsl:text>1</xsl:text>
					</xsl:when>
					<xsl:when test="normalize-space(text()) = &apos;lastDay&apos;">
						<xsl:text>0</xsl:text>
					</xsl:when>
				</xsl:choose>
			</xsl:element>
		</xsl:element>
	</xsl:template>
	
	<xsl:template match="bc:RemoveSponsoree">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>remove_sponsoree</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
				Parameter to indicate whether sponsoree account must be removed from sponsor group upon closure of sponsoree account.
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:element name="Value">
				<xsl:choose>
					<xsl:when test="normalize-space(text()) = &apos;enabled&apos;">
						<xsl:text>1</xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>0</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:element>
		</xsl:element>
	</xsl:template>
	
	<xsl:template match="bc:ProdEndOffsetPlanTransition">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>prod_end_offset_plan_transition</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
				The delay (in days) for cancellation of a 2g service when going to a 3g service or vice-versa during plan transition.
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:variable name="value" select="normalize-space(text())"/>
			<xsl:element name="Value">
					<xsl:value-of select="$value"/>
			</xsl:element>
		</xsl:element>
	</xsl:template>
	
	<xsl:template match="bc:GenerateJournalEpsilon">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>generate_journal_epsilon</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
				Parameter to indicate whether to generate epsilon journal and delta for item
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:element name="Value">
				<xsl:choose>
					<xsl:when test="normalize-space(text()) = &apos;enabled&apos;">
						<xsl:text>1</xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>0</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:element>
		</xsl:element>
	</xsl:template>

	<xsl:template match="bc:ApplyCycleFeeForBillNow">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>apply_cycle_fees_for_bill_now</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
			Parameter to indicate whether to apply cycle fees during bill now.
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:element name="Value">
				<xsl:choose>
					<xsl:when test="normalize-space(text()) = &apos;enabled&apos;">
						<xsl:text>1</xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>0</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:element>
		</xsl:element>
	</xsl:template>

	<xsl:template match="bc:GenerateCorrectiveBillNo">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>generate_corrective_bill_no</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
			Parameter to indicate which pin_sequence to use in order to generate a bill number for a corrective bill. Values are 0 (use pin_sequence for the regular bill) or 1 (use pin_sequence for the corrective bill, this is default).
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:element name="Value">
				<xsl:choose>
					<xsl:when test="normalize-space(text()) = &apos;enabled&apos;">
						<xsl:text>1</xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>0</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:element>
		</xsl:element>
	</xsl:template>

	<xsl:template match="bc:EnableCorrectiveInvoices">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>enable_corrective_invoices</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
			Parameter to indicate that the Corrective Invoicing feature is on. The parameter has two values: 0 - Corrective Invoicing feature is not used (Default), and 1 - Corrective Invoicing feature is used.
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:element name="Value">
				<xsl:choose>
					<xsl:when test="normalize-space(text()) = &apos;enabled&apos;">
						<xsl:text>1</xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>0</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:element>
		</xsl:element>
	</xsl:template>

	<xsl:template match="bc:AllowCorrectivePaidBills">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>allow_corrective_paid_bills</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
			Parameter to indicate whether Corrective Bills can be generated for paid bills. The parameter has two values: 0 - do not allow to generate Corrective Bills for paid bills (default), 1 - allow to generate Corrective Bills for paid bills.
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:element name="Value">
				<xsl:choose>
					<xsl:when test="normalize-space(text()) = &apos;enabled&apos;">
						<xsl:text>1</xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>0</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:element>
		</xsl:element>
	</xsl:template>

	<xsl:template match="bc:RejectPaymentsForPreviousBill">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>reject_payments_for_previous_bill</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
			Parameter to indicate whether to reject a payment if the passing bill_no does not coincide with the bill_no from the last bill. The parameter has two values: 0 - accept payment (default), 1 - reject payment.
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:element name="Value">
				<xsl:choose>
					<xsl:when test="normalize-space(text()) = &apos;enabled&apos;">
						<xsl:text>1</xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>0</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:element>
		</xsl:element>
	</xsl:template>

	<xsl:template match="bc:CorrectiveBillThreshold">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>corrective_bill_threshold</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
			Parameter to indicate the minimum correction amount for which a Corrective bills is allowed. The parameter value is an integer number. Value 0 (default) means that validation for minimum correction amount is not provided.
			</xsl:element>
			<xsl:element name="Type">DECIMAL</xsl:element>
                        <xsl:variable name="value" select="normalize-space(text())"/>
                        <xsl:element name="Value">
                                <xsl:value-of select="$value"/>
                        </xsl:element>
		</xsl:element>
	</xsl:template>

	<xsl:template match="bc:RerateDuringBilling">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>rerate_during_billing</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
			If delayed CDRs borrow against the free resources already consumed by the new cycle CDRs in the delay period, then the new cycle CDRs need to rerated. This parameter indicates whether during billing, the rerating criteria should be evaluated and rerating triggered. By default, this feature is disabled (0)	
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:element name="Value">
				<xsl:choose>
					<xsl:when test="normalize-space(text()) = &apos;enabled&apos;">
						<xsl:text>1</xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>0</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:element>
		</xsl:element>
	</xsl:template>
	
	<xsl:template match="bc:AutoTriggeringLimit">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>auto_triggering_limit</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
			This parameter indicates a count of days before the next billing time when disabling of auto-triggering of billing is allowed. Value 0 indicates to enable auto-triggering.
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:variable name="value" select="normalize-space(text())"/>
			<xsl:element name="Value">
				<xsl:value-of select="$value"/>
			</xsl:element>
		</xsl:element>
	</xsl:template>

	<xsl:template match="bc:SkipCheckForSubordinatesBilled">
                         <xsl:element name="Param">
                        <xsl:element name="Name">
                                <xsl:text>skip_check_for_subordinates_billed</xsl:text>
                        </xsl:element>
                        <xsl:element name="Desc">
			Parameter allows to enable/disable the check, whether billing parent should be billed or not provided Subordinate is billed successfully. If disabled(0) then parent billing will fail if subordinate billing has issues. If enabled(1) then skip the above check.
                        </xsl:element>
                        <xsl:element name="Type">INT</xsl:element>
                        <xsl:variable name="value" select="normalize-space(text())"/>
                        <xsl:element name="Value">
				<xsl:choose>
                                        <xsl:when test="normalize-space(text()) = &apos;disabled&apos;">
                                                <xsl:text>0</xsl:text>
                                        </xsl:when>
					<xsl:when test="normalize-space(text()) = &apos;enabled&apos;">
                                                <xsl:text>1</xsl:text>
					</xsl:when>
                               	</xsl:choose>
                        </xsl:element>
                </xsl:element>
        </xsl:template>
	
	<xsl:template match="bc:ValidateDiscountDependency">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>validate_discount_dependency</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
				Parameter to enable exclusion validation for discounts. Default is 0 (disabled).
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:element name="Value">
				<xsl:choose>
					<xsl:when test="normalize-space(text()) = &apos;disabled&apos;">
						<xsl:text>0</xsl:text>
					</xsl:when>
					<xsl:when test="normalize-space(text()) = &apos;discToDiscExcl&apos;">
						<xsl:text>1</xsl:text>
					</xsl:when>
					<xsl:when test="normalize-space(text()) = &apos;discToPlanExcl&apos;">
						<xsl:text>2</xsl:text>
					</xsl:when>
					<xsl:when test="normalize-space(text()) = &apos;enableBothExcl&apos;">
						<xsl:text>3</xsl:text>
					</xsl:when>
					<xsl:when test="normalize-space(text()) = &apos;disableDiscToPlanExclAndNoPurTimeValidation&apos;">
						<xsl:text>4</xsl:text>
					</xsl:when>
					<xsl:when test="normalize-space(text()) = &apos;enableBothExclAndNoPurTimeValidation&apos;">
						<xsl:text>7</xsl:text>
					</xsl:when>
					<xsl:when test="normalize-space(text()) = &apos;returnOnFirstExcl&apos;">
						<xsl:text>8</xsl:text>
					</xsl:when>
				</xsl:choose>
			</xsl:element>
		</xsl:element>
	</xsl:template>

	<xsl:template match="bc:RolloverCorrectionDuringBilling">
		<xsl:element name="Param">		
			<xsl:element name="Name">
				<xsl:text>rollover_correction_during_billing</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
				If delayed CDRs consume free resources in the delay period, then the rollover that happened at the beginning of the delay period might need to be corrected. This parameter indicates whether during billing, the need for rollover correction should be evaluated and the correction recorded. By default, this feature is disabled (0).
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:element name="Value">
				<xsl:choose>
					<xsl:when test="normalize-space(text()) = &apos;enabled&apos;">
						<xsl:text>1</xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>0</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:element>
		</xsl:element>
	</xsl:template>

	<xsl:template match="bc:EnableARA">
                <xsl:element name="Param">
                        <xsl:element name="Name">
                                <xsl:text>enable_ara</xsl:text>
                        </xsl:element>
                        <xsl:element name="Desc">
                         To turn on ARA for out of cycle billing by setting parameter as 1
                        </xsl:element>
                        <xsl:element name="Type">INT</xsl:element>
                        <xsl:element name="Value">
                                <xsl:choose>
                                        <xsl:when test="normalize-space(text()) = 'enabled'">
                                                <xsl:text>1</xsl:text>
                                        </xsl:when>
                                        <xsl:otherwise>
                                                <xsl:text>0</xsl:text>
                                        </xsl:otherwise>
                                </xsl:choose>
                        </xsl:element>
                </xsl:element>
        </xsl:template>
	
	<xsl:template match="bc:CreateTwoBillNowBillsInDelay">
                <xsl:element name="Param">
                        <xsl:element name="Name">
                                <xsl:text>create_two_bill_now_bills_in_delay</xsl:text>
                        </xsl:element>
                        <xsl:element name="Desc">
                         To enable the creation of 2 Bill_Now bills in delay period set parameter as 1
                        </xsl:element>
                        <xsl:element name="Type">INT</xsl:element>
                        <xsl:element name="Value">
                                <xsl:choose>
                                        <xsl:when test="normalize-space(text()) = 'enabled'">
                                                <xsl:text>1</xsl:text>
                                        </xsl:when>
                                        <xsl:otherwise>
                                                <xsl:text>0</xsl:text>
                                        </xsl:otherwise>
                                </xsl:choose>
                        </xsl:element>
                </xsl:element>
        </xsl:template>

	<xsl:template match="bc:SubBalValidity">
                <xsl:element name="Param">
                        <xsl:element name="Name">
                                <xsl:text>sub_bal_validity</xsl:text>
                        </xsl:element>
                        <xsl:element name="Desc">
                                This parameter is to define the expiration of sub balance bucket for products during line transfer. The parameter values can be 0 (cut), 1 (maintain), 2 (align). Default is 0 (cut).
                        </xsl:element>
                        <xsl:element name="Type">INT</xsl:element>
                        <xsl:element name="Value">
                                <xsl:choose>
                                        <xsl:when test="normalize-space(text()) = &apos;Cut&apos;">
                                                <xsl:text>0</xsl:text>
                                        </xsl:when>
                                        <xsl:when test="normalize-space(text()) = &apos;Maintain&apos;">
                                                <xsl:text>1</xsl:text>
                                        </xsl:when>
                                        <xsl:when test="normalize-space(text()) = &apos;Align&apos;">
                                                <xsl:text>2</xsl:text>
                                        </xsl:when>
                                </xsl:choose>
                        </xsl:element>
                </xsl:element>
        </xsl:template>

	 <xsl:template match="bc:SequentialCycleDiscounting">
                <xsl:element name="Param">
                        <xsl:element name="Name">
                                <xsl:text>sequential_cycle_discounting</xsl:text>
                        </xsl:element>
                        <xsl:element name="Desc">
                         This parameter specifies whether sequential cycle discounting feature is enabled to apply the cycle fee discounts sequentially,possible values are disabled(0) or enabled(1)
                        </xsl:element>
                        <xsl:element name="Type">INT</xsl:element>
                        <xsl:element name="Value">
                                <xsl:choose>
                                        <xsl:when test="normalize-space(text()) = &apos;enabled&apos;">
                                                <xsl:text>1</xsl:text>
                                        </xsl:when>
                                        <xsl:otherwise>
                                                <xsl:text>0</xsl:text>
                                        </xsl:otherwise>
                                </xsl:choose>
                        </xsl:element>
                </xsl:element>
        </xsl:template>
	<xsl:template match="bc:CacheResidencyDistinction">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>cache_residency_distinction</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
                        Parameter to  enable/disable  Object Cache Residency Distinction feature. Default is 0 (disabled)
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:element name="Value">
                                <xsl:choose>
					<xsl:when test="normalize-space(text()) = 'enabled'">
						<xsl:text>1</xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>0</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:element>
		</xsl:element>
	</xsl:template>

	<xsl:template match="bc:DefaultBusinessProfile">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>default_business_profile</xsl:text>
			</xsl:element>
                        <xsl:element name="Desc">
				The default value of business profile is 0(Convergent) 
                        </xsl:element>
			<xsl:element name="Type">STR</xsl:element>
			<xsl:element name="Value">
				<xsl:choose>
					<xsl:when test="normalize-space(text()) = 'Convergent'">
						<xsl:text>Convergent</xsl:text>
					 </xsl:when>
					<xsl:when test="normalize-space(text()) = 'Prepaid'">
						<xsl:text>Prepaid</xsl:text>
					 </xsl:when>
					<xsl:when test="normalize-space(text()) = 'Postpaid'">
						<xsl:text>Postpaid</xsl:text>
					 </xsl:when>
					<xsl:when test="normalize-space(text()) = 'Nonusage'">
						<xsl:text>Nonusage</xsl:text>
					 </xsl:when>
                                        <xsl:when test="normalize-space(text()) = 'SLM'">
                                                <xsl:text>SLM</xsl:text>
                                         </xsl:when>
				</xsl:choose>
			</xsl:element>		
                </xsl:element>
        </xsl:template>
	
	<xsl:template match="bc:AcctCycleDelayPeriod">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>acct_cycle_delay_period</xsl:text>
			</xsl:element>
                        <xsl:element name="Desc">
				The cut off period used by rel to determine the item to be used for rated amount assignment. Default is -1 (Undefined)
                        </xsl:element>
                        <xsl:element name="Type">INT</xsl:element>
                        <xsl:variable name="value" select="text()"></xsl:variable>
                        <xsl:element name="Value">
                                        <xsl:value-of select="$value"></xsl:value-of>
                        </xsl:element>
                </xsl:element>
        </xsl:template>
	
	
        <xsl:template match="bc:GeneralLedgerReporting">
                <xsl:element name="Param">
                        <xsl:element name="Name">
                                <xsl:text>general_ledger_reporting</xsl:text>
                        </xsl:element>
                        <xsl:element name="Desc">
                                Enable/Disable General Ledger Reporting . The parameter values can be 0 (disabled), 1 (enabled).
Default is 1 (enabled).
                        </xsl:element>
                        <xsl:element name="Type">INT</xsl:element>
                        <xsl:element name="Value">
                                <xsl:choose>
                                        <xsl:when test="normalize-space(text()) = 'enabled'">
                                                <xsl:text>1</xsl:text>
                                        </xsl:when>
                                        <xsl:otherwise>
                                                <xsl:text>0</xsl:text>
                                        </xsl:otherwise>
                                </xsl:choose>
                        </xsl:element>
                </xsl:element>
        </xsl:template>


        <xsl:template match="bc:BackdatingPastGlPostedDate">
                <xsl:element name="Param">
                        <xsl:element name="Name">
                                <xsl:text>backdating_past_gl_posted_date</xsl:text>
                        </xsl:element>
                        <xsl:element name="Desc">
                                Parameter to indicate whether backdating past GL posted date is allowed or not. The parameter values can be 0 (disabled) or 1 (enabled). Default is 0 (disabled).
                        </xsl:element>
                        <xsl:element name="Type">INT</xsl:element>
                        <xsl:element name="Value">
                                <xsl:choose>
                                        <xsl:when test="normalize-space(text()) = 'enabled'">
                                                <xsl:text>1</xsl:text>
                                        </xsl:when>
                                        <xsl:otherwise>
                                                <xsl:text>0</xsl:text>
                                        </xsl:otherwise>
                                </xsl:choose>
                        </xsl:element>
                </xsl:element>
        </xsl:template>


        <xsl:template match="bc:CustomJournalUpdate">
                <xsl:element name="Param">
                        <xsl:element name="Name">
                                <xsl:text>custom_journal_update</xsl:text>
                        </xsl:element>
                        <xsl:element name="Desc">
                                Enable/disable customization of general ledger journal entries before they are updated in database. Value can be 0 (disabled) or 1 (enabled). Default is 0.
                        </xsl:element>
                        <xsl:element name="Type">INT</xsl:element>
                        <xsl:element name="Value">
                                <xsl:choose>
                                        <xsl:when test="normalize-space(text()) = 'enabled'">
                                                <xsl:text>1</xsl:text>
                                        </xsl:when>
                                        <xsl:otherwise>
                                                <xsl:text>0</xsl:text>
                                        </xsl:otherwise>
                                </xsl:choose>
                        </xsl:element>
                </xsl:element>
        </xsl:template>
	
	<xsl:template match="bc:PerfAdvancedTuningSettings">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>perf_advanced_tuning_settings</xsl:text>
		</xsl:element>
		<xsl:element name="Desc">
			This parameter is used to tune the behavior of BRM, especially with respect to performance. Bit fields of the integer value are used to specify the
			configuration options. Currently, only bits 0 and 1 are used. Bit 0: Controls setting of item number during billing. 
			Bit 1: Controls updating transfer events during billing.Default value is 0.
			For more details, refer to BRM documentation.
		</xsl:element>
		<xsl:element name="Type">INT</xsl:element>
			<xsl:variable name="value" select="text()"></xsl:variable>
			<xsl:element name="Value">
				<xsl:value-of select="$value"></xsl:value-of>
			</xsl:element>
		</xsl:element>
	</xsl:template>

        <xsl:template match="bc:GenerateBillForChildBillNow">
                <xsl:element name="Param">
                        <xsl:element name="Name">
                                <xsl:text>generate_bill_for_child_billnow</xsl:text>
                </xsl:element>
                <xsl:element name="Desc">
                        This parameter is used to suppress or generate bill objects for non paying child accounts during bill now
                </xsl:element>
                 <xsl:element name="Type">INT</xsl:element>
                 <xsl:element name="Value">
                        <xsl:choose>
                                <xsl:when test="normalize-space(text()) = 'enabled'">
                                        <xsl:text>1</xsl:text>
                                </xsl:when>
                                <xsl:otherwise>
                                        <xsl:text>0</xsl:text>
                                </xsl:otherwise>
                        </xsl:choose>
                 </xsl:element>
                 </xsl:element>
        </xsl:template>

	<xsl:template match="bc:ExchangeRateRefreshInterval">
                <xsl:element name="Param">
                        <xsl:element name="Name">
                                <xsl:text>refresh_exchange_rate</xsl:text>
                        </xsl:element>
                        <xsl:element name="Desc">
                        This parameter indicates the refresh rate interval at which the exchange rates from the BRM database is refreshed during billing.
                        </xsl:element>
                        <xsl:element name="Type">INT</xsl:element>
                        <xsl:variable name="value" select="normalize-space(text())"/>
                        <xsl:element name="Value">
                                <xsl:value-of select="$value"/>
                        </xsl:element>
                </xsl:element>
        </xsl:template>

        <xsl:template match="bc:ApplyARParentCycleFeeFirst">
                <xsl:element name="Param">
                        <xsl:element name="Name">
                                <xsl:text>apply_ar_parent_cycle_fee_first</xsl:text>
                        </xsl:element>
                        <xsl:element name="Desc">
                                Enable/Disable the application of cycle fee for AR parent first, prior to applying for the subordinate. The parameter values can be disabled (0), 1 (enabled). Default is 0 (disabled).
                        </xsl:element>
                        <xsl:element name="Type">INT</xsl:element>
                        <xsl:element name="Value">
                                <xsl:choose>
                                        <xsl:when test="normalize-space(text()) = 'enabled'">
                                                <xsl:text>1</xsl:text>
                                        </xsl:when>
                                        <xsl:otherwise>
                                                <xsl:text>0</xsl:text>
                                        </xsl:otherwise>
                                </xsl:choose>
                        </xsl:element>
                </xsl:element>
        </xsl:template>
	<xsl:template match="bc:SplitSponsorItemByMember">
                         <xsl:element name="Param">
                        <xsl:element name="Name">
                                <xsl:text>split_sponsor_item_by_member</xsl:text>
                        </xsl:element>
                        <xsl:element name="Desc">
				This parameter enables flexible sponsorship item creation. By default it is disabled(0) and only /item/sponsor will be created at account level.When enabled(3) subtype of /item/sponsor may be created and these items will point to member services. Additionally this parameter may only be switched on for onlyRealTime(1) or onlyBatch(2).
                        </xsl:element>
                        <xsl:element name="Type">INT</xsl:element>
                        <xsl:variable name="value" select="normalize-space(text())"/>
                        <xsl:element name="Value">
				<xsl:choose>
					 <xsl:when test="normalize-space(text()) = 'disabled'">
                                                <xsl:text>0</xsl:text>
                                        </xsl:when>
                                        <xsl:when test="normalize-space(text()) = 'enabled'">
                                                <xsl:text>3</xsl:text>
                                        </xsl:when>
                                        <xsl:when test="normalize-space(text()) = 'onlyRealTime'">
                                                <xsl:text>1</xsl:text>
                                        </xsl:when>
                                        <xsl:when test="normalize-space(text()) = 'onlyBatch'">
                                                <xsl:text>2</xsl:text>
                                        </xsl:when>	
                               	</xsl:choose>
                        </xsl:element>
                </xsl:element>
        </xsl:template>

	
	<xsl:template match="bc:StagedBillingFeeProcessing">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>staged_billing_fee_processing</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
			This parameter enables cycle fee processing in parallel by service. Valid values: 0 - Disabled (Default), 1 - Parallel by service, 2 - Parallel by service with enforcement of fee processing before billing, 3 - Parallel by service with fees aggregated to account level item, 4 - Parallel by service with enforcement of fee processing before billing and fees aggregated to account level item.
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:variable name="value" select="normalize-space(text())"/>
			<xsl:element name="Value">
					<xsl:value-of select="$value"/>
			</xsl:element>
		</xsl:element>
	</xsl:template>

	<xsl:template match="bc:NonCurrencyResourceJournaling">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>noncurrency_resource_journaling</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
				Creation of journal for the event with non-currency resource id will be decided based on this parameter value. i.e All or None or +:XXX,YYY or -:XXX,YYY. where XXX and YYY are resource ids. The default value is 'All'.
			</xsl:element>
			<xsl:element name="Type">STR</xsl:element>
			<xsl:element name="Value">
				<xsl:variable name="value">
					<xsl:value-of select="normalize-space(text())"></xsl:value-of>
				</xsl:variable>
				<xsl:choose>
					<xsl:when test="$value">
						<xsl:value-of select="$value" />
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="'All'" />
					</xsl:otherwise>
				</xsl:choose>
			</xsl:element>
		</xsl:element>
	</xsl:template>

	<xsl:template match="bc:ApplyReactivationFee">
		<xsl:element name="Param">
			<xsl:element name="Name">
				<xsl:text>apply_reactivation_fee</xsl:text>
			</xsl:element>
			<xsl:element name="Desc">
			This parameter is for applying the reactivation fee if the reactivation period is more than 90 days in a single year.
			</xsl:element>
			<xsl:element name="Type">INT</xsl:element>
			<xsl:variable name="value" select="normalize-space(text())"/>
			<xsl:element name="Value">
				<xsl:value-of select="$value"/>
			</xsl:element>
		</xsl:element>
	</xsl:template>
</xsl:stylesheet>
