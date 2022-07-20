import java.sql.*;
import java.io.*;
import java.util.*;
import java.util.logging.*;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

import java.nio.charset.Charset;
import java.nio.charset.CharsetEncoder;


class PricingConfig
{
	
	  private String subEvent = "";
	  private String grantEvent = "";
	  private long resId;
	  private String chargePeriod = "";
	  private String isUsageChargeable = "";
	  private String chargeGLId;
	  private String usageGLId;

	  public String getsubEvent() {
	    return subEvent;
	  }

	  public void setsubEvent(String subEvent) {
	    this.subEvent = subEvent;
	  }

	  public String getgrantEvent() {
	    return grantEvent;
	  }

	  public void setgrantEvent(String grantEvent) {
	    this.grantEvent = grantEvent;
	  }

	  public long getresId() {
	    return resId;
	  }

	  public void setresId(long resId) {
	    this.resId = resId;
	  }

	  public String getchargePeriod() {
		    return chargePeriod;
		  }
	  
	  public void setchargePeriod(String chargePeriod ) {
	    this.chargePeriod = chargePeriod;
	  }

	  public String getisUsageChargeable() {
		    return isUsageChargeable;
	  }
	  
	  public void setisUsageChargeable(String isUsageChargeable) {
	    this.isUsageChargeable = isUsageChargeable;
	  }
	  
	  public String getchargeGLId() {
		    return chargeGLId;
		  }

	  public void setchargeGLId(String chargeGLId) {
	    this.chargeGLId = chargeGLId;
	  }
	  
	  public String getUsageGLId() {
		    return usageGLId;
		  }

	  public void setUsageGLId(String usageGLId) {
	    this.usageGLId = usageGLId;
	  }
}

public class PricingUtility {
	public static String ConnectStr = "";
	public static String User = "";
	public static String Pass = "";
	
	public static String UtilOutDir = "";
	public static String UtilLogDir = "";
	public static String UtilInDir = "";
	 
	static CharsetEncoder asciiEncoder = 
	      Charset.forName("US-ASCII").newEncoder(); // or "ISO-8859-1" for ISO Latin 1

	  public static boolean isPureAscii(String v) {
	    return asciiEncoder.canEncode(v);
	  }

	public static void main(String[] args)
    {

		String inputFileName = args[0];
    	//String inputFileName = "new.csv";
        Logger logger = Logger.getLogger("PricingUtilityLog");  
        FileHandler fh; 
    	Properties prop = new Properties();
	InputStream config = null;
	
    	String csvFile = UtilInDir+inputFileName;
        BufferedReader br = null;
        String line = "";
        String cvsSplitBy = ",";
  
        try
        {
	    config = new FileInputStream("config.properties");

	    prop.load(config);
	   
	    ConnectStr = prop.getProperty("hathway.database");
	    User = prop.getProperty("hathway.dbuser");
	    Pass = prop.getProperty("hathway.dbpassword");

	    UtilOutDir = prop.getProperty("hathway.output_file_location");
	    UtilLogDir = prop.getProperty("hathway.log_file_location");
	    UtilInDir = prop.getProperty("hathway.input_file_location");

            fh = new FileHandler(UtilLogDir+inputFileName+".log");  
            logger.addHandler(fh);
            SimpleFormatter formatter = new SimpleFormatter();  
            fh.setFormatter(formatter); 
            logger.info("Database Connection: "+ConnectStr);
            logger.info("Database User: "+User);
            logger.info("Input fie location: "+UtilInDir);
            logger.info("Output fie location: "+UtilOutDir);
            logger.info("Start Processing File: "+inputFileName);
            //logger.info("Start Processing File: "+inputFileName);
        	PrintWriter successFile = new PrintWriter(UtilLogDir+inputFileName+".success");
	        PrintWriter errorFile = new PrintWriter(UtilLogDir+inputFileName+".error");
	        try
	        {
	        	int header = 0;
	    		br = new BufferedReader(new FileReader(csvFile));
	    		while ((line = br.readLine()) != null) 
	    		{
	            	try
	            	{
		    			if (header == 0)
		            	{
		            		//logger.info("Header Record:\n"+line);
		            		header = 1;
		            		continue;
		            	}

						if (!isPureAscii(line.trim()))
						{
							logger.severe("Wrong Encoding"); 
							throw new IllegalArgumentException("Wrong Encoding : Junk Charcter Found");
						}
		                String[] plan_data = line.trim().split(cvsSplitBy, -1);
		                //logger.info((Arrays.toString(plan_data)));
	    		    	//logger.info("Calling Validation");
		                logger.info("Loading Record: ");
		                logger.info((Arrays.toString(plan_data)));
		                logger.info("Validating Data");
	    		    	
		                
		                if (plan_data[0].compareToIgnoreCase("BB") == 0)
		                {
			                dataValidation(logger, plan_data);
		                
			                if (plan_data[3] != null && (plan_data[3].compareToIgnoreCase("UNLIMITED") == 0  
			                		|| plan_data[3].compareToIgnoreCase("LIMITED") == 0
			                		|| plan_data[3].compareToIgnoreCase("LIMITED FUP") == 0
			                		|| plan_data[3].compareToIgnoreCase("UNLIMITED FUP") == 0
			                		))
			                {
			                    createSubscriptionPlan(logger, plan_data);
			                }
			                else if (plan_data[3] != null && ( plan_data[3].compareToIgnoreCase("RENTAL") == 0
			                		|| plan_data[3].compareToIgnoreCase("RENTAL-DEPOSIT") == 0
			                		|| plan_data[3].compareToIgnoreCase("OUTRIGHT") == 0)
			                	)
			                {
			                    createHardwarePlan(logger, plan_data); //HW Plan
			                }
			                successFile.println(line);
			                //insert data in pricing master table
			                insertPricingData(logger, plan_data);
		                }
		                else if (plan_data[0].compareToIgnoreCase("CATV") == 0)
		                {
		            	    String payTerm = plan_data[3].trim();
		            	    String payUnit = plan_data[4].trim();
		            	    String planName = plan_data[5].trim();
		            	    String charge = plan_data[8].trim();		            		
		                	catvDataValidation(logger, plan_data);
		                	
		            	    String payTermStr = "";
		            	    if (payUnit.compareToIgnoreCase("month") == 0)
		            	    {
		            	    	if(payTerm.compareToIgnoreCase("1") == 0)
			            		{
			            			payTermStr = "MONTHLY";
			            		}
			            		else if(payTerm.compareToIgnoreCase("3") == 0)
			            		{
			            			payTermStr = "QUATERLY";
			            		}
			            		else if(payTerm.compareToIgnoreCase("6") == 0)
			            		{
			            			payTermStr = "HALF YEARLY";
			            		}
			            		else if(payTerm.compareToIgnoreCase("12") == 0)
			            		{
			            			payTermStr = "YEARLY";
			            		}
		            	    }
		            		else if (payUnit.compareToIgnoreCase("day") == 0)
		            		{
		            			payTermStr = payTerm+" DAYS";
		            		}
		            		
		            		String description = planName+" ,RS "+charge+" "+payTermStr+" CHARGE PLUS TAX";

		            		
			                createCATVPlan(logger, plan_data, description);
			                successFile.println(line);
			                //insert data in pricing master table
			                insertCATVPricingData(logger, plan_data, description);
		                }
	            	}
	    		    catch (Exception e)
	    		    {
	    		    	//logger.info("XML Generation failed");
	    		    	logger.info("XML Generation failed");
	    		    	errorFile.println(line+","+e.getMessage());
	    		    	logger.severe(e.getMessage());
	    		    	//logger.info("System MSG: "+e.getMessage());
	    		    	continue;
	    		    }
	    		}
	        }
	        catch (FileNotFoundException e) {
	        	//logger.info("File Not Found : " + e.getMessage());
	        	successFile.close();
	        	errorFile.close();
	        	logger.severe("File Not Found");
	        	logger.severe(e.getMessage());
	        } 
	        catch (IOException e) {
	        	//logger.info("Input Output Exception : " + e.getMessage());
	        	successFile.close();
	        	errorFile.close();
	        	//e.printStackTrace();
	        	logger.severe("Input Output Exception");
	        	logger.severe(e.getMessage());
	        }
	        catch(Exception exception){
	                //logger.info("Error : " + exception.getMessage());
	            	successFile.close();
	            	errorFile.close();
	                //exception.printStackTrace();
	            	logger.severe("Error in Main");
	            	logger.severe(exception.getMessage());
	        }
        	successFile.close();
        	errorFile.close();
        }
        catch(Exception e)
        {
        	//e.printStackTrace();
        	logger.severe("Input Output Exception");
        	logger.severe(e.getMessage());
        	//logger.info("File Not Found");
        }
	finally
	{
		if (config != null)
		{
			try 
			{
				config.close();
			}
			catch (IOException e) 
			{
				e.printStackTrace();
			}
		}
	}
    }

    public static void createSubscriptionPlan (Logger logger, String [] plan_data)
    {
        String semicolonSplitBy = ";";
        String colonSplitBy = ":";

        try
		{        	
		    String serviceType = plan_data[0].trim().toUpperCase();
		    String technology = plan_data[1].trim();
		    String payType = plan_data[2].trim().toUpperCase();
		    String planType = plan_data[3].trim().toUpperCase();
		    String planPayTerm = plan_data[4].trim();
		    String planName = plan_data[5].trim();
		    String provTag = plan_data[6].trim();
		    String cityWiseCharges = plan_data[7].trim();
		    String freeMBStr = plan_data[8].trim();
		    String chargePerMB =  plan_data[9].trim();
		    String chargeProrate = plan_data[10].trim();
		    String mbProrate = plan_data[11].trim();
		    String subPriority =  plan_data[12].trim();
		    String grantPriority =  plan_data[13].trim();
		    String usagePriority = plan_data[14].trim();
		    String planSubType = "";
		    String planValidDays = "";
		    if (plan_data.length == 17)
		    {
		    	planSubType = plan_data[15].trim();
		    	planValidDays = plan_data[16].trim();
		    }

			PricingConfig pc = new PricingConfig();

			try{
				pc = GetPricingConfig(logger, serviceType, payType, planType, planPayTerm, technology);
			}
			catch(Exception e)
			{
				throw new IllegalArgumentException(e.getMessage());
			}
			
	        String dealName = planName+"-DEAL";
	        String subPrdName = planName+"-SUBSCRIPTION";
	        String grantPrdName = planName+"-GRANT";
	        String usagePrdName = planName+"-USAGE";
	        long 	freeMb = 0;
	        long	limitedFUP = 0;
	        if (planType.compareToIgnoreCase("LIMITED FUP") == 0)
	        {
	        	String[] grantedFreebies = freeMBStr.split(";",-1);
	        	freeMb = Long.parseLong(grantedFreebies[0].trim());
	        	limitedFUP = Long.parseLong(grantedFreebies[1].trim());
	        }
	        else if (planType.compareToIgnoreCase("UNLIMITED") != 0)
	        {
	        	freeMb = Long.parseLong(freeMBStr);
	        }
	        String chargePerMb = chargePerMB;
	        double chargePerMbD = Double.parseDouble(chargePerMB);
			String service = "";
			if (serviceType.compareToIgnoreCase("BB") == 0)
			{
				service="/service/telco/broadband";
			}
			else
			{
				service="/service/catv";
			}
	        
			String usageEvent = "/event/session/telco/broadband";
			String currency = "356";
			String usageTrackRes = "1000103";
			String excessUsageTrackRes = "1000105";
			String FupRes = "1000104";
			String subEvent = "";
			String grantEvent = "";
			long resId;
			String chargePeriod = "";
			String isUsageChargeable = "";
			long chargeGLId = 0;

			subEvent = pc.getsubEvent();
			grantEvent = pc.getgrantEvent();
			resId = pc.getresId();
			chargePeriod = pc.getchargePeriod();
			isUsageChargeable = pc.getisUsageChargeable();
			String usageGLId = pc.getUsageGLId();
			if (pc.getchargeGLId() != null && pc.getchargeGLId().compareToIgnoreCase("") != 0)
			{
				chargeGLId = Long.parseLong(pc.getchargeGLId().trim());
			}
			DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder docBuilder = docFactory.newDocumentBuilder();
	
			// root elements
			Document doc = docBuilder.newDocument();
			
			Element rootElement = doc.createElement("price_list");
			doc.appendChild(rootElement);
			
			Attr attr = doc.createAttribute("version");
			attr.setValue("7.2");
			rootElement.setAttributeNode(attr);
			
			attr = doc.createAttribute("xmlns:xsi");
			attr.setValue("http://www.w3.org/2001/XMLSchema-instance");
			rootElement.setAttributeNode(attr);
			
			attr = doc.createAttribute("xsi:noNamespaceSchemaLocation");
			attr.setValue("price_list.xsd");
			rootElement.setAttributeNode(attr);
			Element plan = doc.createElement("plan");
			rootElement.appendChild(plan);
			
			attr = doc.createAttribute("ondemand_billing");
			attr.setValue("no");
			plan.setAttributeNode(attr);
	
			Element element = doc.createElement("plan_name");
			element.appendChild(doc.createTextNode(planName));
			plan.appendChild(element);
			
			element = doc.createElement("description");
			element.appendChild(doc.createTextNode(planName));
			plan.appendChild(element);
			
			Element serDeal = doc.createElement("service_deal");
			plan.appendChild(serDeal);
			
			element = doc.createElement("service_name");
			element.appendChild(doc.createTextNode(service));
			serDeal.appendChild(element);
			
			element = doc.createElement("deal_name");
			element.appendChild(doc.createTextNode(dealName));
			serDeal.appendChild(element);
			
			element = doc.createElement("bal_info_index");
			element.appendChild(doc.createTextNode("0"));
			serDeal.appendChild(element);
			
			element = doc.createElement("subscription_index");
			element.appendChild(doc.createTextNode("0"));
			serDeal.appendChild(element);
			
			Element deal = doc.createElement("deal");
			rootElement.appendChild(deal);
			
			attr = doc.createAttribute("customization_flag");
			attr.setValue("optional");
			deal.setAttributeNode(attr);
	
			attr = doc.createAttribute("ondemand_billing");
			attr.setValue("no");
			deal.setAttributeNode(attr);
			
			
			element = doc.createElement("deal_name");
			element.appendChild(doc.createTextNode(dealName));
			deal.appendChild(element);
			
			element = doc.createElement("description");
			element.appendChild(doc.createTextNode(dealName));
			deal.appendChild(element);
			
			
			element = doc.createElement("permitted");
			element.appendChild(doc.createTextNode(service));
			deal.appendChild(element);
			
			Element dealPrd1 = doc.createElement("deal_product");
			deal.appendChild(dealPrd1);
			
			attr = doc.createAttribute("status");
			attr.setValue("inactive");
			dealPrd1.setAttributeNode(attr);
			
			element = doc.createElement("product_name");
			element.appendChild(doc.createTextNode(grantPrdName));
			dealPrd1.appendChild(element);
			element = doc.createElement("product_code");
			element.appendChild(doc.createTextNode(grantPrdName));
			dealPrd1.appendChild(element);
			element = doc.createElement("quantity");
			element.appendChild(doc.createTextNode("1.0"));
			dealPrd1.appendChild(element);
			
			
			if (planSubType != null && planSubType.compareToIgnoreCase("") != 0 && planSubType.compareToIgnoreCase("DEMO") == 0){
				element = doc.createElement("purchase_end");
				attr = doc.createAttribute("unit");
				attr.setValue("day");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planValidDays));
				dealPrd1.appendChild(element);
			}
			else if (payType.compareToIgnoreCase("PREPAID")==0)
			{
				element = doc.createElement("purchase_end");
				attr = doc.createAttribute("unit");
				attr.setValue("month");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planPayTerm));
				dealPrd1.appendChild(element);
			}
			
			element = doc.createElement("purchase_discount");
			element.appendChild(doc.createTextNode("0.0"));
			dealPrd1.appendChild(element);
			
			if (planSubType != null && planSubType.compareToIgnoreCase("") != 0 && planSubType.compareToIgnoreCase("DEMO") == 0){
				element = doc.createElement("cycle_end");
				attr = doc.createAttribute("unit");
				attr.setValue("day");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planValidDays));
				dealPrd1.appendChild(element);
			}
			else if (payType.compareToIgnoreCase("PREPAID")==0)
			{
				element = doc.createElement("cycle_end");
				attr = doc.createAttribute("unit");
				attr.setValue("month");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planPayTerm));
				dealPrd1.appendChild(element);
			}

			element = doc.createElement("cycle_discount");
			element.appendChild(doc.createTextNode("0.0"));
			dealPrd1.appendChild(element);
			

			if (planSubType != null && planSubType.compareToIgnoreCase("") != 0 && planSubType.compareToIgnoreCase("DEMO") == 0){
				element = doc.createElement("usage_end");
				attr = doc.createAttribute("unit");
				attr.setValue("day");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planValidDays));
				dealPrd1.appendChild(element);
			}
			else if (payType.compareToIgnoreCase("PREPAID")==0)
			{
				element = doc.createElement("usage_end");
				attr = doc.createAttribute("unit");
				attr.setValue("month");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planPayTerm));
				dealPrd1.appendChild(element);
			}

			element = doc.createElement("usage_discount");
			element.appendChild(doc.createTextNode("0.0"));
			dealPrd1.appendChild(element);
	
			element = doc.createElement("status_flag");
			element.appendChild(doc.createTextNode("0"));
			dealPrd1.appendChild(element);
			
			Element dealPrd2 = doc.createElement("deal_product");
			deal.appendChild(dealPrd2);
			
			attr = doc.createAttribute("status");
			attr.setValue("inactive");			
			dealPrd2.setAttributeNode(attr);
			
			element = doc.createElement("product_name");
			element.appendChild(doc.createTextNode(usagePrdName));
			dealPrd2.appendChild(element);
			element = doc.createElement("product_code");
			element.appendChild(doc.createTextNode(usagePrdName));
			dealPrd2.appendChild(element);
			element = doc.createElement("quantity");
			element.appendChild(doc.createTextNode("1.0"));
			dealPrd2.appendChild(element);
			
			if (planSubType != null && planSubType.compareToIgnoreCase("") != 0 && planSubType.compareToIgnoreCase("DEMO") == 0){
				element = doc.createElement("purchase_end");
				attr = doc.createAttribute("unit");
				attr.setValue("day");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planValidDays));
				dealPrd2.appendChild(element);
			}
			else if (payType.compareToIgnoreCase("PREPAID")==0)
			{
				element = doc.createElement("purchase_end");
				attr = doc.createAttribute("unit");
				attr.setValue("month");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planPayTerm));
				dealPrd2.appendChild(element);
			}
			element = doc.createElement("purchase_discount");
			element.appendChild(doc.createTextNode("0.0"));
			dealPrd2.appendChild(element);
			
			if (planSubType != null && planSubType.compareToIgnoreCase("") != 0 && planSubType.compareToIgnoreCase("DEMO") == 0){
				element = doc.createElement("cycle_end");
				attr = doc.createAttribute("unit");
				attr.setValue("day");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planValidDays));
				dealPrd2.appendChild(element);
			}
			else if (payType.compareToIgnoreCase("PREPAID")==0)
			{
				element = doc.createElement("cycle_end");
				attr = doc.createAttribute("unit");
				attr.setValue("month");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planPayTerm));
				dealPrd2.appendChild(element);
			}

			element = doc.createElement("cycle_discount");
			element.appendChild(doc.createTextNode("0.0"));
			dealPrd2.appendChild(element);
			
			if (planSubType != null && planSubType.compareToIgnoreCase("") != 0 && planSubType.compareToIgnoreCase("DEMO") == 0){
				element = doc.createElement("usage_end");
				attr = doc.createAttribute("unit");
				attr.setValue("day");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planValidDays));
				dealPrd2.appendChild(element);
			}
			else if (payType.compareToIgnoreCase("PREPAID")==0)
			{
				element = doc.createElement("usage_end");
				attr = doc.createAttribute("unit");
				attr.setValue("month");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planPayTerm));
				dealPrd2.appendChild(element);
			}
			
			element = doc.createElement("usage_discount");
			element.appendChild(doc.createTextNode("0.0"));
			dealPrd2.appendChild(element);
	
			element = doc.createElement("status_flag");
			element.appendChild(doc.createTextNode("0"));
			dealPrd2.appendChild(element);
			
			Element dealPrd3 = doc.createElement("deal_product");
			deal.appendChild(dealPrd3);
			
			attr = doc.createAttribute("status");
			attr.setValue("inactive");			
			dealPrd3.setAttributeNode(attr);
			
			element = doc.createElement("product_name");
			element.appendChild(doc.createTextNode(subPrdName));
			dealPrd3.appendChild(element);
			element = doc.createElement("product_code");
			element.appendChild(doc.createTextNode(subPrdName));
			dealPrd3.appendChild(element);
			element = doc.createElement("quantity");
			element.appendChild(doc.createTextNode("1.0"));
			dealPrd3.appendChild(element);
			
			if (planSubType != null && planSubType.compareToIgnoreCase("") != 0 && planSubType.compareToIgnoreCase("DEMO") == 0){
				element = doc.createElement("purchase_end");
				attr = doc.createAttribute("unit");
				attr.setValue("day");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planValidDays));
				dealPrd3.appendChild(element);
			}
			else if (payType.compareToIgnoreCase("PREPAID")==0)
			{
				element = doc.createElement("purchase_end");
				attr = doc.createAttribute("unit");
				attr.setValue("month");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planPayTerm));
				dealPrd3.appendChild(element);
			}

			element = doc.createElement("purchase_discount");
			element.appendChild(doc.createTextNode("0.0"));
			dealPrd3.appendChild(element);
			
			if (planSubType != null && planSubType.compareToIgnoreCase("") != 0 && planSubType.compareToIgnoreCase("DEMO") == 0){
				element = doc.createElement("cycle_end");
				attr = doc.createAttribute("unit");
				attr.setValue("day");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planValidDays));
				dealPrd3.appendChild(element);
			}
			else if (payType.compareToIgnoreCase("PREPAID")==0)
			{
				element = doc.createElement("cycle_end");
				attr = doc.createAttribute("unit");
				attr.setValue("month");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planPayTerm));
				dealPrd3.appendChild(element);
			}
			element = doc.createElement("cycle_discount");
			element.appendChild(doc.createTextNode("0.0"));
			dealPrd3.appendChild(element);

			if (planSubType != null && planSubType.compareToIgnoreCase("") != 0 && planSubType.compareToIgnoreCase("DEMO") == 0){
				element = doc.createElement("usage_end");
				attr = doc.createAttribute("unit");
				attr.setValue("day");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planValidDays));
				dealPrd3.appendChild(element);
			}
			else if (payType.compareToIgnoreCase("PREPAID")==0)
			{
				
				element = doc.createElement("usage_end");
				attr = doc.createAttribute("unit");
				attr.setValue("month");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planPayTerm));
				dealPrd3.appendChild(element);
			}
			element = doc.createElement("usage_discount");
			element.appendChild(doc.createTextNode("0.0"));
			dealPrd3.appendChild(element);
	
			element = doc.createElement("status_flag");
			element.appendChild(doc.createTextNode("0"));
			dealPrd3.appendChild(element);
			
			//grant product
			Element grantPrdTag = doc.createElement("product");
			rootElement.appendChild(grantPrdTag);
			
			attr = doc.createAttribute("partial");
			attr.setValue("no");			
			grantPrdTag.setAttributeNode(attr);
			attr = doc.createAttribute("type");
			attr.setValue("subscription");			
			grantPrdTag.setAttributeNode(attr);
			
			element = doc.createElement("product_name");
			element.appendChild(doc.createTextNode(grantPrdName));
			grantPrdTag.appendChild(element);
			
			element = doc.createElement("product_code");
			element.appendChild(doc.createTextNode(grantPrdName));
			grantPrdTag.appendChild(element);
			
			element = doc.createElement("priority");
			element.appendChild(doc.createTextNode(grantPriority));
			grantPrdTag.appendChild(element);
			
			element = doc.createElement("description");
			element.appendChild(doc.createTextNode(grantPrdName));
			grantPrdTag.appendChild(element);
			
			element = doc.createElement("permitted");
			element.appendChild(doc.createTextNode(service));
			grantPrdTag.appendChild(element);
			
			
			Element eventRatMap = doc.createElement("event_rating_map");
			attr = doc.createAttribute("incr_unit");
			attr.setValue("none");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("min_unit");
			attr.setValue("none");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("rounding_rule");
			attr.setValue("nearest");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("timezone_mode");
			attr.setValue("event");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("tod_mode");
			attr.setValue("start_time");			
			eventRatMap.setAttributeNode(attr);
			grantPrdTag.appendChild(eventRatMap);
			
			
			element = doc.createElement("event_type");
			element.appendChild(doc.createTextNode(grantEvent));
			eventRatMap.appendChild(element);
	
			element = doc.createElement("rum_name");
			element.appendChild(doc.createTextNode("Occurrence"));
			eventRatMap.appendChild(element);
			
			element = doc.createElement("min_quantity");
			element.appendChild(doc.createTextNode("1.0"));
			eventRatMap.appendChild(element);
			
			element = doc.createElement("incr_quantity");
			element.appendChild(doc.createTextNode("1.0"));
			eventRatMap.appendChild(element);
			
			element = doc.createElement("rate_plan_name");
			element.appendChild(doc.createTextNode(planName+" GRANT "+planPayTerm));
			eventRatMap.appendChild(element);
			
			Element grantRatePlan = doc.createElement("rate_plan");
			attr = doc.createAttribute("advance_billing_unit");
			attr.setValue("day");			
			grantRatePlan.setAttributeNode(attr);
			attr = doc.createAttribute("tax_when");
			attr.setValue("never");		
			grantRatePlan.setAttributeNode(attr);
			eventRatMap.appendChild(grantRatePlan);
			element = doc.createElement("rate_plan_name");
			element.appendChild(doc.createTextNode(planName+" GRANT "+planPayTerm));	
			grantRatePlan.appendChild(element);
			
			element = doc.createElement("currency_id");
			element.appendChild(doc.createTextNode(currency));
			grantRatePlan.appendChild(element);
			
			element = doc.createElement("event_type");
			element.appendChild(doc.createTextNode(grantEvent));
			grantRatePlan.appendChild(element);
			
			element = doc.createElement("advance_billing_offset");
			element.appendChild(doc.createTextNode("0"));
			grantRatePlan.appendChild(element);
			
			element = doc.createElement("cycle_fee_flags");
			element.appendChild(doc.createTextNode("0"));
			grantRatePlan.appendChild(element);
			
			Element grantRateTier = doc.createElement("rate_tier");
			attr = doc.createAttribute("date_range_type");
			attr.setValue("absolute");			
			grantRateTier.setAttributeNode(attr);
			grantRatePlan.appendChild(grantRateTier);
			
			element = doc.createElement("rate_tier_name");
			element.appendChild(doc.createTextNode("Tier 1"));
			grantRateTier.appendChild(element);
			
			element = doc.createElement("priority");
			element.appendChild(doc.createTextNode("0.0"));
			grantRateTier.appendChild(element);
			
			
			Element grantRate = doc.createElement("rate");
			if (mbProrate.compareToIgnoreCase("TRUE") != 0)
			{
				attr = doc.createAttribute("prorate_first");
				attr.setValue("full");			
				grantRate.setAttributeNode(attr);
				attr = doc.createAttribute("prorate_last");
				attr.setValue("full");			
				grantRate.setAttributeNode(attr);
			}
			else
			{
				attr = doc.createAttribute("prorate_first");
				attr.setValue("prorate");			
				grantRate.setAttributeNode(attr);
				attr = doc.createAttribute("prorate_last");
				attr.setValue("prorate");			
				grantRate.setAttributeNode(attr);
			}
			attr = doc.createAttribute("step_type");
			attr.setValue("total_quantity_rated");			
			grantRate.setAttributeNode(attr);
			attr = doc.createAttribute("type");
			attr.setValue("normal");			
			grantRate.setAttributeNode(attr);
			grantRateTier.appendChild(grantRate);
			
			element = doc.createElement("description");
			element.appendChild(doc.createTextNode("Rate 1"));
			grantRate.appendChild(element);
			
			element = doc.createElement("step_resource_id");
			
			if (planType.compareToIgnoreCase("LIMITED") == 0)
				element.appendChild(doc.createTextNode(resId+""));
			else
				element.appendChild(doc.createTextNode("0"));
			
			grantRate.appendChild(element);
			
			Element quantTier = doc.createElement("quantity_tier");
			grantRate.appendChild(quantTier);
			Element balImpact;
			if (planType.compareToIgnoreCase("UNLIMITED") != 0)
			{
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("flag");
				if (mbProrate.compareToIgnoreCase("TRUE") != 0)
				{
					attr.setValue("discountable grantable");			
				}
				else
				{
					attr.setValue("discountable proratable grantable");	
				}
				balImpact.setAttributeNode(attr);
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(resId+""));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode("0"));
				balImpact.appendChild(element);
				

				if (planSubType != null && planSubType.compareToIgnoreCase("") != 0 && planSubType.compareToIgnoreCase("DEMO") == 0){
					element = doc.createElement("fixed_amount");
					element.appendChild(doc.createTextNode((-1)*freeMb+""));
					balImpact.appendChild(element);
					

					element = doc.createElement("scaled_amount");
					element.appendChild(doc.createTextNode("0.0"));
					balImpact.appendChild(element);
					element = doc.createElement("relative_end_offset");
					attr = doc.createAttribute("unit");
					attr.setValue("day");			
					element.setAttributeNode(attr);
					element.appendChild(doc.createTextNode(planValidDays));
				}
				else if (payType.compareToIgnoreCase("PREPAID")==0){
					element = doc.createElement("fixed_amount");
					element.appendChild(doc.createTextNode((-1)*freeMb+""));
					balImpact.appendChild(element);
					

					element = doc.createElement("scaled_amount");
					element.appendChild(doc.createTextNode("0.0"));
					balImpact.appendChild(element);
					element = doc.createElement("relative_end_offset");
					attr = doc.createAttribute("unit");
					attr.setValue("month");			
					element.setAttributeNode(attr);
					if(planType.compareToIgnoreCase("UNLIMITED FUP") == 0)
					{
						element.appendChild(doc.createTextNode("1"));
					}
					else
					{
						element.appendChild(doc.createTextNode(planPayTerm));
					}
					
				}
	        	else {
					element = doc.createElement("fixed_amount");
					element.appendChild(doc.createTextNode("0.0"));
					balImpact.appendChild(element);
					

					element = doc.createElement("scaled_amount");
					element.appendChild(doc.createTextNode((-1)*freeMb+""));
					balImpact.appendChild(element);
					element = doc.createElement("relative_end_offset");
					attr = doc.createAttribute("unit");
					attr.setValue("acct_cycle");			
					element.setAttributeNode(attr);
					element.appendChild(doc.createTextNode("1"));
	        	}
				balImpact.appendChild(element);
			}
			
			balImpact = doc.createElement("balance_impact");
			attr = doc.createAttribute("flag");
			attr.setValue("grantable");			
			balImpact.setAttributeNode(attr);
			attr = doc.createAttribute("scaled_unit");
			attr.setValue("none");			
			balImpact.setAttributeNode(attr);
			quantTier.appendChild(balImpact);
			
			element = doc.createElement("resource_id");
			element.appendChild(doc.createTextNode(usageTrackRes));
			balImpact.appendChild(element);
			
			element = doc.createElement("glid");
			element.appendChild(doc.createTextNode("0"));
			balImpact.appendChild(element);
			
			element = doc.createElement("fixed_amount");
			element.appendChild(doc.createTextNode("0.0"));
			balImpact.appendChild(element);
			
			element = doc.createElement("scaled_amount");
			element.appendChild(doc.createTextNode("0.0"));
			balImpact.appendChild(element);
			
			element = doc.createElement("relative_end_offset");
			attr = doc.createAttribute("unit");
			
			if (planSubType != null && planSubType.compareToIgnoreCase("") != 0 && planSubType.compareToIgnoreCase("DEMO") == 0){
				attr.setValue("day");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode(planValidDays));
			}
			else if (payType.compareToIgnoreCase("PREPAID")==0)
			{
				attr.setValue("month");
				element.setAttributeNode(attr);
				
				if(planType.compareToIgnoreCase("UNLIMITED FUP") == 0)
				{
					element.appendChild(doc.createTextNode("1"));
				}
				else
				{
					element.appendChild(doc.createTextNode(planPayTerm));
				}
			}
			else{
				attr.setValue("acct_cycle");
				element.setAttributeNode(attr);
				element.appendChild(doc.createTextNode("1"));
			}
			balImpact.appendChild(element);
			
			if (planType.compareToIgnoreCase("LIMITED FUP") == 0)
			{
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("flag");
				attr.setValue("discountable proratable grantable");			
				balImpact.setAttributeNode(attr);
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(FupRes));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode("0"));
				balImpact.appendChild(element);
				
				if (planSubType != null && planSubType.compareToIgnoreCase("") != 0 && planSubType.compareToIgnoreCase("DEMO") == 0){
					element = doc.createElement("fixed_amount");
					element.appendChild(doc.createTextNode((-1)*limitedFUP+""));
					balImpact.appendChild(element);
					

					element = doc.createElement("scaled_amount");
					element.appendChild(doc.createTextNode("0.0"));
					balImpact.appendChild(element);
					element = doc.createElement("relative_end_offset");
					attr = doc.createAttribute("unit");
					attr.setValue("day");			
					element.setAttributeNode(attr);
					element.appendChild(doc.createTextNode(planValidDays));
				}
				else if (payType.compareToIgnoreCase("PREPAID")==0)
				{
					element = doc.createElement("fixed_amount");
					element.appendChild(doc.createTextNode((-1)*limitedFUP+""));
					balImpact.appendChild(element);
					
					element = doc.createElement("scaled_amount");
					element.appendChild(doc.createTextNode("0.0"));
					balImpact.appendChild(element);
					
					element = doc.createElement("relative_end_offset");
					attr = doc.createAttribute("unit");
					attr.setValue("month");
					element.setAttributeNode(attr);
					element.appendChild(doc.createTextNode(planPayTerm));
				}
				else{
					element = doc.createElement("fixed_amount");
					element.appendChild(doc.createTextNode("0.0"));
					balImpact.appendChild(element);
					
					element = doc.createElement("scaled_amount");
					element.appendChild(doc.createTextNode((-1)*limitedFUP+""));
					balImpact.appendChild(element);
					
					element = doc.createElement("relative_end_offset");
					attr = doc.createAttribute("unit");
					attr.setValue("acct_cycle");
					element.setAttributeNode(attr);
					element.appendChild(doc.createTextNode("1"));
				}
				balImpact.appendChild(element);
			}
			if (chargePerMbD > 0)
			{	    		
	    		balImpact = doc.createElement("balance_impact");
	    		attr = doc.createAttribute("flag");
				attr.setValue("grantable");			
				balImpact.setAttributeNode(attr);
	    		attr = doc.createAttribute("scaled_unit");
	    		attr.setValue("none");			
	    		balImpact.setAttributeNode(attr);
	    		quantTier.appendChild(balImpact);
	    		
	    		element = doc.createElement("resource_id");
	    		element.appendChild(doc.createTextNode(excessUsageTrackRes));
	    		balImpact.appendChild(element);
	    		
	    		element = doc.createElement("glid");
	    		element.appendChild(doc.createTextNode("0"));
	    		balImpact.appendChild(element);
	    		
	    		element = doc.createElement("fixed_amount");
	    		element.appendChild(doc.createTextNode("0.0"));
	    		balImpact.appendChild(element);
	    		
	    		element = doc.createElement("scaled_amount");
	    		element.appendChild(doc.createTextNode("0.0"));
	    		balImpact.appendChild(element);
	    		
	    		element = doc.createElement("relative_end_offset");
				attr = doc.createAttribute("unit");
				
				if (planSubType != null && planSubType.compareToIgnoreCase("") != 0 && planSubType.compareToIgnoreCase("DEMO") == 0){
					attr.setValue("day");			
					element.setAttributeNode(attr);
					element.appendChild(doc.createTextNode(planValidDays));
				}
				if (payType.compareToIgnoreCase("PREPAID")==0)
				{
					attr.setValue("month");
					element.setAttributeNode(attr);
					element.appendChild(doc.createTextNode(planPayTerm));
				}
				else{
					attr.setValue("acct_cycle");
					element.setAttributeNode(attr);
					element.appendChild(doc.createTextNode("1"));
				}
				balImpact.appendChild(element);
			}

			//usage product
			Element usagePrdTag = doc.createElement("product");
			rootElement.appendChild(usagePrdTag);
			
			attr = doc.createAttribute("partial");
			attr.setValue("no");			
			usagePrdTag.setAttributeNode(attr);
			attr = doc.createAttribute("type");
			attr.setValue("subscription");			
			usagePrdTag.setAttributeNode(attr);
			
			element = doc.createElement("product_name");
			element.appendChild(doc.createTextNode(usagePrdName));
			usagePrdTag.appendChild(element);
			
			element = doc.createElement("product_code");
			element.appendChild(doc.createTextNode(usagePrdName));
			usagePrdTag.appendChild(element);
			
			element = doc.createElement("priority");
			element.appendChild(doc.createTextNode(usagePriority));
			usagePrdTag.appendChild(element);
			
			element = doc.createElement("description");
			element.appendChild(doc.createTextNode(usagePrdName));
			usagePrdTag.appendChild(element);
			
			element = doc.createElement("permitted");
			element.appendChild(doc.createTextNode(service));
			usagePrdTag.appendChild(element);
			
			eventRatMap = doc.createElement("event_rating_map");
			attr = doc.createAttribute("incr_unit");
			attr.setValue("none");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("min_unit");
			attr.setValue("none");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("rounding_rule");
			attr.setValue("nearest");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("timezone_mode");
			attr.setValue("event");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("tod_mode");
			attr.setValue("start_time");			
			eventRatMap.setAttributeNode(attr);
			usagePrdTag.appendChild(eventRatMap);
			
			
			element = doc.createElement("event_type");
			element.appendChild(doc.createTextNode(usageEvent));
			eventRatMap.appendChild(element);
	
			element = doc.createElement("rum_name");
			element.appendChild(doc.createTextNode("UsageVolume"));
			eventRatMap.appendChild(element);
			
			element = doc.createElement("min_quantity");
			element.appendChild(doc.createTextNode("1.0"));
			eventRatMap.appendChild(element);
			
			element = doc.createElement("incr_quantity");
			element.appendChild(doc.createTextNode("1.0"));
			eventRatMap.appendChild(element);
			
			element = doc.createElement("rate_plan_name");
			element.appendChild(doc.createTextNode("Broadband Session"));
			eventRatMap.appendChild(element);
			
			Element usageRatePlan = doc.createElement("rate_plan");
			attr = doc.createAttribute("advance_billing_unit");
			attr.setValue("day");			
			usageRatePlan.setAttributeNode(attr);
			attr = doc.createAttribute("tax_when");
			attr.setValue("now");		
			usageRatePlan.setAttributeNode(attr);
			eventRatMap.appendChild(usageRatePlan);
			element = doc.createElement("rate_plan_name");
			element.appendChild(doc.createTextNode("Broadband Session"));
			usageRatePlan.appendChild(element);
			
			element = doc.createElement("currency_id");
			element.appendChild(doc.createTextNode(currency));
			usageRatePlan.appendChild(element);
			
			element = doc.createElement("event_type");
			element.appendChild(doc.createTextNode("/event/session/telco/broadband"));
			usageRatePlan.appendChild(element);
			
			element = doc.createElement("tax_code");
			element.appendChild(doc.createTextNode("MSO_Service_Tax"));
			usageRatePlan.appendChild(element);
			
			element = doc.createElement("advance_billing_offset");
			element.appendChild(doc.createTextNode("0"));
			usageRatePlan.appendChild(element);
			
			element = doc.createElement("cycle_fee_flags");
			element.appendChild(doc.createTextNode("0"));
			usageRatePlan.appendChild(element);
			
			Element usageRateTier = doc.createElement("rate_tier");
			attr = doc.createAttribute("date_range_type");
			attr.setValue("absolute");			
			usageRateTier.setAttributeNode(attr);
			usageRatePlan.appendChild(usageRateTier);
			
			element = doc.createElement("rate_tier_name");
			element.appendChild(doc.createTextNode("Usage-Tier"));
			usageRateTier.appendChild(element);
			
			element = doc.createElement("priority");
			element.appendChild(doc.createTextNode("1.0"));
			usageRateTier.appendChild(element);
			
			
			Element usageRate = doc.createElement("rate");
			attr = doc.createAttribute("prorate_first");
			attr.setValue("prorate");			
			usageRate.setAttributeNode(attr);
			attr = doc.createAttribute("prorate_last");
			attr.setValue("full");			
			usageRate.setAttributeNode(attr);
			
			if (planType.compareToIgnoreCase("UNLIMITED") != 0)
			{
				attr = doc.createAttribute("step_type");
				attr.setValue("resource_quantity");		
			}
			else {
				attr = doc.createAttribute("step_type");
				attr.setValue("total_quantity_rated");
			}
			usageRate.setAttributeNode(attr);
			attr = doc.createAttribute("type");
			attr.setValue("normal");			
			usageRate.setAttributeNode(attr);
			usageRateTier.appendChild(usageRate);
			
			element = doc.createElement("description");
			element.appendChild(doc.createTextNode("Usage-Rate"));
			usageRate.appendChild(element);
			
			element = doc.createElement("step_resource_id");
			if (planType.compareToIgnoreCase("UNLIMITED") != 0)
			{
				element.appendChild(doc.createTextNode(resId+""));
			}
			else
			{
				element.appendChild(doc.createTextNode("0"));
			}
			usageRate.appendChild(element);
			
			quantTier = doc.createElement("quantity_tier");
			usageRate.appendChild(quantTier);

			if (planType.compareToIgnoreCase("UNLIMITED")!= 0 && planType.compareToIgnoreCase("LIMITED FUP") != 0)
			{
				element = doc.createElement("step_max");
				element.appendChild(doc.createTextNode("0.0"));
				quantTier.appendChild(element);
				
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(resId+""));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode("0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode("1.0"));
				balImpact.appendChild(element);
				
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(usageTrackRes));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode("0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode("1.0"));
				balImpact.appendChild(element);
				
				quantTier = doc.createElement("quantity_tier");
				usageRate.appendChild(quantTier);
				
				element = doc.createElement("step_min");
				element.appendChild(doc.createTextNode("0.0"));
				quantTier.appendChild(element);
		
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(usageTrackRes));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode("0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode("1.0"));
				balImpact.appendChild(element);
				
				if (chargePerMbD > 0)
				{
					balImpact = doc.createElement("balance_impact");
		    		attr = doc.createAttribute("scaled_unit");
		    		attr.setValue("none");			
		    		balImpact.setAttributeNode(attr);
		    		quantTier.appendChild(balImpact);
		    		
		    		element = doc.createElement("resource_id");
		    		element.appendChild(doc.createTextNode(currency));
		    		balImpact.appendChild(element);
		    		
		    		element = doc.createElement("glid");
		    		element.appendChild(doc.createTextNode(usageGLId.trim()));  //usage glid
		    		balImpact.appendChild(element);
		    		
		    		element = doc.createElement("fixed_amount");
		    		element.appendChild(doc.createTextNode("0.0"));
		    		balImpact.appendChild(element);
		    		
		    		element = doc.createElement("scaled_amount");
		    		element.appendChild(doc.createTextNode(chargePerMb));
		    		balImpact.appendChild(element);
		    		
		    		balImpact = doc.createElement("balance_impact");
		    		attr = doc.createAttribute("scaled_unit");
		    		attr.setValue("none");			
		    		balImpact.setAttributeNode(attr);
		    		quantTier.appendChild(balImpact);
		    		
		    		element = doc.createElement("resource_id");
		    		element.appendChild(doc.createTextNode(excessUsageTrackRes));
		    		balImpact.appendChild(element);
		    		
		    		element = doc.createElement("glid");
		    		element.appendChild(doc.createTextNode("0"));
		    		balImpact.appendChild(element);
		    		
		    		element = doc.createElement("fixed_amount");
		    		element.appendChild(doc.createTextNode("0.0"));
		    		balImpact.appendChild(element);
		    		
		    		element = doc.createElement("scaled_amount");
		    		element.appendChild(doc.createTextNode("1.0"));
		    		balImpact.appendChild(element);
					
				}
			}
			else if (planType.compareToIgnoreCase("LIMITED FUP") ==0 )
			{
				
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(resId+""));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode("0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode("1.0"));
				balImpact.appendChild(element);
				
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(FupRes+""));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode("0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode("1.0"));
				balImpact.appendChild(element);
				
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(usageTrackRes+""));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode("0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode("1.0"));
				balImpact.appendChild(element);
				
				
				usageRateTier = doc.createElement("rate_tier");
				attr = doc.createAttribute("date_range_type");
				attr.setValue("absolute");			
				usageRateTier.setAttributeNode(attr);
				usageRatePlan.appendChild(usageRateTier);
				
				element = doc.createElement("rate_tier_name");
				element.appendChild(doc.createTextNode("Usage-Tier 2"));
				usageRateTier.appendChild(element);
				
				element = doc.createElement("priority");
				element.appendChild(doc.createTextNode("0.0"));
				usageRateTier.appendChild(element);
				
				
				usageRate = doc.createElement("rate");
				attr = doc.createAttribute("prorate_first");
				attr.setValue("prorate");			
				usageRate.setAttributeNode(attr);
				attr = doc.createAttribute("prorate_last");
				attr.setValue("full");			
				usageRate.setAttributeNode(attr);
				attr = doc.createAttribute("step_type");
				attr.setValue("resource_quantity");		
				usageRate.setAttributeNode(attr);
				attr = doc.createAttribute("type");
				attr.setValue("normal");			
				usageRate.setAttributeNode(attr);
				usageRateTier.appendChild(usageRate);
				
				element = doc.createElement("description");
				element.appendChild(doc.createTextNode("Usage-Rate 2"));
				usageRate.appendChild(element);
				
				element = doc.createElement("step_resource_id");
				element.appendChild(doc.createTextNode(FupRes+""));
				usageRate.appendChild(element);
				
				quantTier = doc.createElement("quantity_tier");
				usageRate.appendChild(quantTier);
				
				element = doc.createElement("step_max");
				element.appendChild(doc.createTextNode("0.0"));
				quantTier.appendChild(element);
				
				
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(currency));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode(usageGLId.trim()));		//usage glid
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode(chargePerMb));
				balImpact.appendChild(element);
				
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(FupRes+""));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode("0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode("1.0"));
				balImpact.appendChild(element);
				
				
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(usageTrackRes+""));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode("0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode("1.0"));
				balImpact.appendChild(element);
				
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(excessUsageTrackRes));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode("0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode("1.0"));
				balImpact.appendChild(element);
				
				quantTier = doc.createElement("quantity_tier");
				usageRate.appendChild(quantTier);
				
				element = doc.createElement("step_min");
				element.appendChild(doc.createTextNode("0.0"));
				quantTier.appendChild(element);
				
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(currency));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode(usageGLId.trim()));   		//usage glid
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode(chargePerMb));
				balImpact.appendChild(element);
				
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(usageTrackRes));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode("0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode("1.0"));
				balImpact.appendChild(element);
				
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(excessUsageTrackRes));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode("0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode("1.0"));
				balImpact.appendChild(element);
			}
			else
			{
				balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(usageTrackRes));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode("0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode("1.0"));
				balImpact.appendChild(element);
			}
			//subscription product
			Element subsPrdTag = doc.createElement("product");
			rootElement.appendChild(subsPrdTag);
			
			attr = doc.createAttribute("partial");
			attr.setValue("no");			
			subsPrdTag.setAttributeNode(attr);
			attr = doc.createAttribute("type");
			attr.setValue("subscription");			
			subsPrdTag.setAttributeNode(attr);
			
			element = doc.createElement("product_name");
			element.appendChild(doc.createTextNode(subPrdName));
			subsPrdTag.appendChild(element);
			
			element = doc.createElement("product_code");
			element.appendChild(doc.createTextNode(subPrdName));
			subsPrdTag.appendChild(element);
			
			element = doc.createElement("priority");
			element.appendChild(doc.createTextNode(subPriority));
			subsPrdTag.appendChild(element);
			
			element = doc.createElement("description");
			element.appendChild(doc.createTextNode(subPrdName));
			subsPrdTag.appendChild(element);
			
			element = doc.createElement("permitted");
			element.appendChild(doc.createTextNode(service));
			subsPrdTag.appendChild(element);
			
			element = doc.createElement("provisioning");
			element.appendChild(doc.createTextNode(provTag));
			subsPrdTag.appendChild(element);
			
			eventRatMap = doc.createElement("event_rating_map");
			attr = doc.createAttribute("incr_unit");
			attr.setValue("none");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("min_unit");
			attr.setValue("none");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("rounding_rule");
			attr.setValue("nearest");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("timezone_mode");
			attr.setValue("event");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("tod_mode");
			attr.setValue("start_time");			
			eventRatMap.setAttributeNode(attr);
			subsPrdTag.appendChild(eventRatMap);    	            		
			
			element = doc.createElement("event_type");
			element.appendChild(doc.createTextNode(subEvent)); //Susbcription event
			eventRatMap.appendChild(element);
	
			element = doc.createElement("rum_name");
			element.appendChild(doc.createTextNode("Occurrence"));
			eventRatMap.appendChild(element);
			
			element = doc.createElement("min_quantity");
			element.appendChild(doc.createTextNode("1.0"));
			eventRatMap.appendChild(element);
			
			element = doc.createElement("incr_quantity");
			element.appendChild(doc.createTextNode("1.0"));
			eventRatMap.appendChild(element);
			
			Element SubsRatePlanSelect = doc.createElement("rate_plan_selector");
			eventRatMap.appendChild(SubsRatePlanSelect);
			
			element = doc.createElement("rate_plan_selector_name");
			element.appendChild(doc.createTextNode(planName+" - PLAN SELECTOR"));
			SubsRatePlanSelect.appendChild(element);
			
			Element SubsSelector = doc.createElement("selector");
			SubsRatePlanSelect.appendChild(SubsSelector);
			
			Element column = doc.createElement("column");
			attr = doc.createAttribute("operator");
			attr.setValue("equal");			
			column.setAttributeNode(attr);
			attr = doc.createAttribute("separator");
			attr.setValue("");			
			column.setAttributeNode(attr);
			SubsSelector.appendChild(column);
			
			element = doc.createElement("field_name");
			element.appendChild(doc.createTextNode("SERVICE.MSO_FLD_BB_INFO.PIN_FLD_CITY"));
			column.appendChild(element);
			
			column = doc.createElement("column");
			attr = doc.createAttribute("operator");
			attr.setValue("equal");			
			column.setAttributeNode(attr);
			attr = doc.createAttribute("separator");
			attr.setValue("");			
			column.setAttributeNode(attr);
			SubsSelector.appendChild(column);
			
			element = doc.createElement("field_name");
			element.appendChild(doc.createTextNode("SERVICE.MSO_FLD_BB_INFO.PIN_FLD_BILL_WHEN"));
			column.appendChild(element);
	
			//Loop
			//logger.info(cityWiseCharges);
			String[] citywisedata = cityWiseCharges.split(semicolonSplitBy, -1);
			String[] city = new String[100];
			String[] payTerm = new String[100];
			String[] price = new String[100];
			String[] ratePlanName = new String[100];
			Element ValRange1;
			Element ValRange2;
			 
			int i=0;
			int arrayLength = citywisedata.length;
			//logger.info("arrayLength"+arrayLength);
			while (i < arrayLength)
			{
				//logger.info(citywisedata[i]);
				String[] citydata = citywisedata[i].trim().split(colonSplitBy, -1);
			
				city [i] = citydata [0].trim().toUpperCase();
				payTerm [i] = citydata [1].trim();
				price [i] = citydata [2].trim();
				ratePlanName [i] = city[i]+"_"+payTerm[i]+"_"+planName;
				
				ValRange1 = doc.createElement("value_range");
				SubsSelector.appendChild(ValRange1);
	    		
	    		element = doc.createElement("value");
	    		element.appendChild(doc.createTextNode(city[i]));	
	    		ValRange1.appendChild(element);
	    		
	    		ValRange2 = doc.createElement("value_range");
	    		ValRange1.appendChild(ValRange2);
	    		
	    		element = doc.createElement("value");
	    		element.appendChild(doc.createTextNode(payTerm[i]));
	    		ValRange2.appendChild(element);
	    		
	    		
	    		element = doc.createElement("rate_plan_name");
	    		element.appendChild(doc.createTextNode(ratePlanName [i]));	
	    		ValRange2.appendChild(element);
	    		
	    		element = doc.createElement("impact_category");
	    		element.appendChild(doc.createTextNode("default"));	
	    		ValRange2.appendChild(element);
				i++;
			}
			//Loop End
			int j = 0;
			Element subRatePlan;
			Element subRateTier;
			Element subRate;
			while (j < arrayLength)
			{
				subRatePlan = doc.createElement("rate_plan");
	    		attr = doc.createAttribute("advance_billing_unit");
	    		attr.setValue("day");			
	    		subRatePlan.setAttributeNode(attr);
	    		attr = doc.createAttribute("tax_when");
	    		attr.setValue("now");		
	    		subRatePlan.setAttributeNode(attr);
	    		eventRatMap.appendChild(subRatePlan);
	    		
	    		element = doc.createElement("rate_plan_name");
	    		element.appendChild(doc.createTextNode(ratePlanName [j]));
	    		subRatePlan.appendChild(element);
	    		
	    		element = doc.createElement("currency_id");
	    		element.appendChild(doc.createTextNode(currency));
	    		subRatePlan.appendChild(element);
	    		
	    		element = doc.createElement("event_type");
	    		element.appendChild(doc.createTextNode(subEvent));
	    		subRatePlan.appendChild(element);
	    		
	    		element = doc.createElement("tax_code");
	    		element.appendChild(doc.createTextNode("MSO_Service_Tax"));
	    		subRatePlan.appendChild(element);
	    		
	    		element = doc.createElement("advance_billing_offset");
	    		element.appendChild(doc.createTextNode("0"));
	    		subRatePlan.appendChild(element);
	    		
	    		element = doc.createElement("cycle_fee_flags");
	    		element.appendChild(doc.createTextNode("0"));
	    		subRatePlan.appendChild(element);
	    		
	    		subRateTier = doc.createElement("rate_tier");
	    		attr = doc.createAttribute("date_range_type");
	    		attr.setValue("absolute");			
	    		subRateTier.setAttributeNode(attr);
	    		subRatePlan.appendChild(subRateTier);
	    		
	    		element = doc.createElement("rate_tier_name");
	    		element.appendChild(doc.createTextNode("New Rate Tier"));
	    		subRateTier.appendChild(element);
	    		
	    		element = doc.createElement("priority");
	    		element.appendChild(doc.createTextNode("0.0"));
	    		subRateTier.appendChild(element);
	    		
	    		
	    		subRate = doc.createElement("rate");
	    		
	    		if(chargeProrate.compareToIgnoreCase("TRUE")!= 0)
	    		{
		    		attr = doc.createAttribute("prorate_first");
		    		attr.setValue("full");			//proration rule
		    		subRate.setAttributeNode(attr);
		    		attr = doc.createAttribute("prorate_last");
		    		attr.setValue("full");			//proration rule
		    		subRate.setAttributeNode(attr);
	    		}
	    		else
	    		{
		    		attr = doc.createAttribute("prorate_first");
		    		attr.setValue("prorate");			//proration rule
		    		subRate.setAttributeNode(attr);
		    		attr = doc.createAttribute("prorate_last");
		    		attr.setValue("prorate");			//proration rule
		    		subRate.setAttributeNode(attr);
	    		}
	    		attr = doc.createAttribute("step_type");
	    		attr.setValue("total_quantity_rated");			
	    		subRate.setAttributeNode(attr);
	    		attr = doc.createAttribute("type");
	    		attr.setValue("normal");			
	    		subRate.setAttributeNode(attr);
	    		subRateTier.appendChild(subRate);
	    		
	    		element = doc.createElement("description");
	    		element.appendChild(doc.createTextNode("New Rate Tier"));
	    		subRate.appendChild(element);
	    		
	    		element = doc.createElement("step_resource_id");
	    		element.appendChild(doc.createTextNode("0"));
	    		subRate.appendChild(element);
	    		
	    		quantTier = doc.createElement("quantity_tier");
	    		subRate.appendChild(quantTier);
	    		
	    		balImpact = doc.createElement("balance_impact");
	    		attr = doc.createAttribute("flag");
	    		if(chargeProrate.compareToIgnoreCase("TRUE")!= 0)
	    			attr.setValue("discountable");		
	    		else
	    			attr.setValue("discountable proratable");
	    		
	    		balImpact.setAttributeNode(attr);
	    		attr = doc.createAttribute("scaled_unit");
	    		attr.setValue("none");
	    		balImpact.setAttributeNode(attr);
	    		quantTier.appendChild(balImpact);
	    		
	    		element = doc.createElement("resource_id");
	    		element.appendChild(doc.createTextNode(currency));
	    		balImpact.appendChild(element);
	    		
	    		element = doc.createElement("glid");
	    		element.appendChild(doc.createTextNode(chargeGLId+""));
	    		balImpact.appendChild(element);
	    		
	    		if (payType.compareToIgnoreCase("PREPAID") == 0)
	    		{
		    		element = doc.createElement("fixed_amount");
		    		element.appendChild(doc.createTextNode(price [j]));
		    		balImpact.appendChild(element);
		    		
		    		element = doc.createElement("scaled_amount");
		    		element.appendChild(doc.createTextNode("0.0"));
		    		balImpact.appendChild(element);
	    		}
	    		else
	    		{
		    		element = doc.createElement("fixed_amount");
		    		element.appendChild(doc.createTextNode("0.0"));
		    		balImpact.appendChild(element);
		    		
		    		element = doc.createElement("scaled_amount");
		    		element.appendChild(doc.createTextNode(price [j]));
		    		balImpact.appendChild(element);
	    		}
				j++;
			}
			
			TransformerFactory transformerFactory = TransformerFactory.newInstance();
			Transformer transformer = transformerFactory.newTransformer();
			DOMSource source = new DOMSource(doc);
			//StreamResult result = new StreamResult(new File("D:\\CSV_File\\"+planName+".xml"));
			String fileName = planName.replace(" ", "_");
			StreamResult result = new StreamResult(new File(UtilOutDir+fileName+".xml"));
			transformer.transform(source, result);
	
			//logger.info("Pricing XML Generated!");
			logger.info("Pricing XML Generated");
			return;
	    }
		catch (TransformerException tfe) 
		{
			throw new IllegalArgumentException(tfe.getMessage());
			//tfe.printStackTrace();
		}
		catch (ParserConfigurationException pce) 
		{
			throw new IllegalArgumentException(pce.getMessage());
			//pce.printStackTrace();
		} 
		catch(Exception exception){
			throw new IllegalArgumentException(exception.getMessage());
	        //exception.printStackTrace();
		}
    }   

	public static PricingConfig GetPricingConfig(Logger logger, String servType, String payType, String planType, String payTerm, String technology)
	{
		Connection mainConnection = null;
	    ResultSet resultSet = null;
	    Statement statement = null;
	    PricingConfig pc = new PricingConfig();
		try{
		    Class.forName("oracle.jdbc.driver.OracleDriver");
			mainConnection=DriverManager.getConnection(ConnectStr, User, Pass);
			
			String getPricingConfig = "";
			if (planType.compareToIgnoreCase("OUTRIGHT") == 0){
				getPricingConfig= "select * from pricing_master_config_t where upper(SERVICE_TYPE) = '"+servType.toUpperCase()+
											"' and upper(TECHNOLOGY) = '"+technology.toUpperCase()+
											"' and upper(PLAN_TYPE) = '"+planType.toUpperCase()+"'";
			}
			else if ((planType.compareToIgnoreCase("RENTAL") == 0 || planType.compareToIgnoreCase("RENTAL-DEPOSIT") == 0)){
				getPricingConfig= "select * from pricing_master_config_t where upper(SERVICE_TYPE) = '"+servType.toUpperCase()+
										"' and upper(PAY_TERM) = '"+payTerm.toUpperCase()+						
										"' and upper(TECHNOLOGY) = '"+technology.toUpperCase()+
										"' and upper(PLAN_TYPE) = '"+planType.toUpperCase()+"'";
			}
			else
			{
				getPricingConfig= "select * from pricing_master_config_t where upper(SERVICE_TYPE) = '"+servType.toUpperCase()+
											"' and upper(PAY_TYPE) = '"+payType.toUpperCase()+
											"' and upper(PAY_TERM) = '"+payTerm.toUpperCase()+
											"' and upper(TECHNOLOGY) = '"+technology.toUpperCase()+
											"' and upper(PLAN_TYPE) = '"+planType.toUpperCase()+"'";
			}
			logger.info(getPricingConfig);
			logger.info("Fetching Configuration..");
			Class.forName("oracle.jdbc.driver.OracleDriver");
			statement = mainConnection.createStatement();
			resultSet = statement.executeQuery(getPricingConfig);
			if(resultSet.next())
			{
					pc.setsubEvent(resultSet.getString(6));
					pc.setgrantEvent(resultSet.getString(7));
					pc.setresId(resultSet.getLong(8));
					pc.setchargeGLId(resultSet.getString(9));
					pc.setchargePeriod(resultSet.getString(10));
					pc.setisUsageChargeable(resultSet.getString(11));
					pc.setUsageGLId(resultSet.getString(12));
			}
			else
			{
				//logger.info("Configuration not found");
				logger.severe("Configuration not found");
				throw new IllegalArgumentException("Config not found for combination");
			}
		}
		catch (Exception e)
		{
			//logger.info("Here go custom msg");
			//logger.info("Error : " + e.getMessage());
			logger.severe(e.getMessage());
	        throw new IllegalArgumentException("Config not found for combination");
		}
	    finally
	    {
	    	try
            {
                resultSet.close();
                statement.close();

                if (mainConnection != null)
                {
                	mainConnection.close();
                }
            }
            catch (Exception ignored)
            {}
	    }
	    return pc;
	}
	
	public static void insertPricingData(Logger logger, String [] plan_data)
	{
		Connection mainConnection = null;
	    Statement statement = null;
	    ResultSet resultSet = null;
		try{
			String planType = plan_data[3].trim();
		    Class.forName("oracle.jdbc.driver.OracleDriver");
			mainConnection=DriverManager.getConnection(ConnectStr, User, Pass);
			String insertStatement = "";
		    if(planType != null && planType.compareToIgnoreCase("") != 0 
	    			&& (planType.compareToIgnoreCase("UNLIMITED FUP") == 0
				    	|| planType.compareToIgnoreCase("LIMITED FUP") == 0
				    	|| planType.compareToIgnoreCase("UNLIMITED") == 0
				    	|| planType.compareToIgnoreCase("LIMITED") == 0))
			{
				insertStatement= "insert into pricing_master_t values (plan_id.NEXTVAL,'"+plan_data[0].trim()+"','"+
				plan_data[1].trim().toUpperCase()+"','"+plan_data[2].trim().toUpperCase()+"','"+plan_data[3].trim().toUpperCase()+"','"+
				plan_data[4].trim()+"','"+plan_data[5].trim()+"','"+plan_data[6].trim()+"','"+
				plan_data[8].trim()+"','"+plan_data[9].trim()+"','"+plan_data[11].trim()+"','"+
				plan_data[10].trim()+"','"+plan_data[12].trim()+"','"+
				plan_data[13].trim()+"','"+plan_data[14].trim()+"','0')";
			}
			else
			{
				insertStatement= "insert into pricing_master_t values (plan_id.NEXTVAL,'"+plan_data[0].trim()+"','"+
				plan_data[1].trim()+"','"+plan_data[2].trim().toUpperCase()+"','"+plan_data[3].trim().toUpperCase()+"','"+
				plan_data[4].trim()+"','"+plan_data[5].trim()+"','"+plan_data[6].trim()+"','"+
				plan_data[8].trim()+"','"+plan_data[9].trim()+"','"+plan_data[11].trim()+"','"+
				plan_data[10].trim()+"','"+plan_data[12].trim()+"','"+
				plan_data[13].trim()+"','','0')";
			}
		   // logger.info(insertStatement);
			statement = mainConnection.createStatement();
			statement.execute(insertStatement);
			
			String GetPlanId="select plan_id from pricing_master_t where status = 0 and plan_name = '"+plan_data[5].trim()+"'";
			//logger.info(GetPlanId);
			resultSet = statement.executeQuery(GetPlanId);
			long plan_id = 0;
			if(resultSet.next())
			{
					plan_id = resultSet.getLong(1);
			}
			else
			{
				//logger.info("Configuration not found");
				logger.severe("Plan Data not found");
				throw new IllegalArgumentException("Plan Data not found");
			}
			
			String[] citywisedata = plan_data[7].trim().split(";", -1);
			String[] city = new String[100];
			String[] payTerm = new String[100];
			String[] price = new String[100];
			 
			int i=0;
			int arrayLength = citywisedata.length;
			//logger.info("arrayLength"+arrayLength);
			while (i < arrayLength)
			{
                if (plan_data[3] != null && (plan_data[3].compareToIgnoreCase("UNLIMITED") == 0  
                		|| plan_data[3].compareToIgnoreCase("LIMITED") == 0
                		|| plan_data[3].compareToIgnoreCase("LIMITED FUP") == 0
                		|| plan_data[3].compareToIgnoreCase("UNLIMITED FUP") == 0
                		))
                {
					String[] citydata = citywisedata[i].trim().split(":", -1);
					city [i] = citydata [0].trim();
					payTerm [i] = citydata [1].trim();
					price [i] = citydata [2].trim();
					String insertSubData = "insert into pricing_master_data_t values ("+plan_id+",'"+"SUBSCRIPTION"+"','"+city [i]+"','"+payTerm [i]
					                        +"','"+price [i]+"')";
					//logger.info(insertSubData);
					statement.execute(insertSubData);
                }
                else if (plan_data[3] != null && ( plan_data[3].compareToIgnoreCase("RENTAL") == 0
                		|| plan_data[3].compareToIgnoreCase("OUTRIGHT") == 0)
                	)
                {
					String priceHw = citywisedata[i];
					String insertSubData = "insert into pricing_master_data_t values ("+plan_id+",'"+plan_data[3].trim()+"','"+"*"+"','"+plan_data[4].trim()
					                        +"','"+priceHw+"')";
					//logger.info(insertSubData);
					statement.execute(insertSubData);
                }
                else if (plan_data[3].compareToIgnoreCase("RENTAL-DEPOSIT") == 0)
                {
                	String plan_type = "";
					if ( i == 0)
					{
						plan_type = "RENTAL";
					}
					else
					{
						plan_type = "DEPOSIT";
					}
                	String priceHw = citywisedata[i];
					String insertSubData = "insert into pricing_master_data_t values ("+plan_id+",'"+plan_type+"','"+"*"+"','"+plan_data[4].trim()
					                        +"','"+priceHw+"')";
					//logger.info(insertSubData);
					statement.execute(insertSubData);
                }
				i++;
			}			
		}
		catch (Exception e)
		{
			//logger.info("Here go custom msg");
			//logger.info("Error : " + e.getMessage());
			logger.severe(e.getMessage());
	        throw new IllegalArgumentException("Error inserting data in Table");
		}
	    finally
	    {
	    	try
            {
                statement.close();

                if (mainConnection != null)
                {
                	mainConnection.close();
                }
            }
            catch (Exception ignored)
            {}
	    }
	}
	
	public static int planValidation(Logger logger, String planName, String ServType)
	{
		Connection mainConnection = null;
	    ResultSet resultSet = null;
	    Statement statement = null;
	    int plan_count = -1;
		try{
		    Class.forName("oracle.jdbc.driver.OracleDriver");
			mainConnection=DriverManager.getConnection(ConnectStr, User, Pass);
			
			String getPlanIsp= "select count(*) from pricing_master_t where plan_name = '"+planName+"' and status = 0";
			String getPlanCatv= "select count(*) from pricing_catv_master_t where plan_name = '"+planName+"' and status = 0";
			String getPlan= "select count(*) from plan_t where name = '"+planName+"'";
			logger.info("Validating Plan..");
			Class.forName("oracle.jdbc.driver.OracleDriver");
			statement = mainConnection.createStatement();
			if (ServType.compareToIgnoreCase("BB") == 0)
			{
				resultSet = statement.executeQuery(getPlanIsp);
			}
			else
			{
				resultSet = statement.executeQuery(getPlanCatv);
			}
			if(resultSet.next())
			{
					plan_count = resultSet.getInt(1);
					if (plan_count > 0)
						return plan_count;
			}
			else
			{
				logger.severe("Error in searching plan in master");
				throw new IllegalArgumentException("Error in searching plan in master");
			}
			resultSet = statement.executeQuery(getPlan);
			if(resultSet.next())
			{
					plan_count = resultSet.getInt(1);
					if (plan_count > 0)
						return plan_count;
			}
			else
			{
				logger.severe("Error in searching plan in Plans");
				throw new IllegalArgumentException("Error in searching plan in Plans");
			}
		}
		catch (Exception e)
		{
			logger.severe(e.getMessage());
	        throw new IllegalArgumentException("Error in searching plan");
		}
	    finally
	    {
	    	try
            {
                resultSet.close();
                statement.close();

                if (mainConnection != null)
                {
                	mainConnection.close();
                }
            }
            catch (Exception ignored)
            {}
	    }
	    return plan_count;
	}
	
	public static int cityValidation(Logger logger, String city)
	{
		Connection mainConnection = null;
	    ResultSet resultSet = null;
	    Statement statement = null;
	    int city_count = -1;
		try{
		    Class.forName("oracle.jdbc.driver.OracleDriver");
			mainConnection=DriverManager.getConnection(ConnectStr, User, Pass);
			
			String getcity= "select count(*) from pricing_city_config_t where city = '"+city+"'";
			logger.info("Validating City..");
			Class.forName("oracle.jdbc.driver.OracleDriver");
			statement = mainConnection.createStatement();
			resultSet = statement.executeQuery(getcity);
			if(resultSet.next())
			{
					city_count = resultSet.getInt(1);
					return city_count;
			}
			else
			{
				logger.severe("Error in searching city in master");
				throw new IllegalArgumentException("Error in searching city in master");
			}
		}
		catch (Exception e)
		{
			logger.severe(e.getMessage());
	        throw new IllegalArgumentException("Error in searching city");
		}
	    finally
	    {
	    	try
            {
                resultSet.close();
                statement.close();

                if (mainConnection != null)
                {
                	mainConnection.close();
                }
            }
            catch (Exception ignored)
            {}
	    }
	}
	
    public static void createHardwarePlan (Logger logger, String [] plan_data)
    {                
        try
		{
		    String serviceType = plan_data[0].trim();
		    String technology = plan_data[1].trim();
		    String payType = plan_data[2].trim();
		    String planType = plan_data[3].trim();
		    String planPayTerm = plan_data[4].trim();
		    String planName = plan_data[5].trim();
		    String charge = plan_data[7].trim();
		    String rentProrate = plan_data[10].trim();
		    String rentPriority =  plan_data[12].trim();
		    String depOutPriority =  plan_data[13].trim();
	        
	        String dealName = planName+"-DEAL";
	        String rentPrdName = "";
	        String depOutPrdName = "";
	        String rentCharge = "";
	        String depOutCharge = "";
	        

	        if (planType.compareToIgnoreCase("RENTAL-DEPOSIT") == 0)
	        {
	        	String []chargeArray = charge.trim().split(";",-1);
	        	rentCharge = new String(chargeArray[0]);
	        	depOutCharge = new String(chargeArray[1]);
	        }
	        else if (planType.compareToIgnoreCase("RENTAL") == 0)
	        {
	        	rentCharge = new String(charge);
	        }
	        else
	        {
	        	depOutCharge = new String(charge);
	        }
	        	        
			String service = "";
			if (serviceType.compareToIgnoreCase("BB") == 0)
			{
				service="/service/telco/broadband";
			}
			else{
				service="/service/catv";
			}

			String currency = "356";
			String rentEvent = "";
			String rentGLId = "";
			String depOutGLId = "";
			PricingConfig pc = new PricingConfig();
			try{
				pc = GetPricingConfig(logger, serviceType, payType, planType, planPayTerm, technology);
			}
			catch(Exception e)
			{
				throw new IllegalArgumentException(e.getMessage());
			}
			rentEvent = pc.getsubEvent();
			String GLIdStr = pc.getchargeGLId();
			String depOutEvent = "/event/billing/product/fee/purchase";

	        if (planType.compareToIgnoreCase("RENTAL-DEPOSIT") == 0)
	        {
	        	String [] GLArray = GLIdStr.trim().split(";",-1);
	        	rentGLId = new String(GLArray[0].trim());
	        	depOutGLId = new String(GLArray[1].trim());
	        }
	        else if (planType.compareToIgnoreCase("RENTAL") == 0)
	        {
	        	rentGLId = new String(GLIdStr.trim());
	        }
	        else
	        {
	        	depOutGLId = new String(GLIdStr.trim());
	        }
	        
	        if (planType.compareToIgnoreCase("RENTAL-DEPOSIT") == 0)
	        {
	        	rentPrdName = planName+"-RENTAL";
	        	depOutPrdName = planName+"-DEPOSIT";
	        }
	        else if (planType.compareToIgnoreCase("RENTAL") == 0)
	        {
	        	rentPrdName = planName+"-RENTAL";
	        }
	        else
	        {
	        	depOutPrdName = planName+"-OUTRIGHT";
	        }
			
			DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder docBuilder = docFactory.newDocumentBuilder();
	
			// root elements
			Document doc = docBuilder.newDocument();
			
			Element rootElement = doc.createElement("price_list");
			doc.appendChild(rootElement);
			
			Attr attr = doc.createAttribute("version");
			attr.setValue("7.2");
			rootElement.setAttributeNode(attr);
			
			attr = doc.createAttribute("xmlns:xsi");
			attr.setValue("http://www.w3.org/2001/XMLSchema-instance");
			rootElement.setAttributeNode(attr);
			
			attr = doc.createAttribute("xsi:noNamespaceSchemaLocation");
			attr.setValue("price_list.xsd");
			rootElement.setAttributeNode(attr);
			Element plan = doc.createElement("plan");
			rootElement.appendChild(plan);
			
			attr = doc.createAttribute("ondemand_billing");
			attr.setValue("no");
			plan.setAttributeNode(attr);
	
			Element element = doc.createElement("plan_name");
			element.appendChild(doc.createTextNode(planName));
			plan.appendChild(element);
			
			element = doc.createElement("description");
			element.appendChild(doc.createTextNode(planName));
			plan.appendChild(element);
			
			Element serDeal = doc.createElement("service_deal");
			plan.appendChild(serDeal);
			
			element = doc.createElement("service_name");
			element.appendChild(doc.createTextNode(service));
			serDeal.appendChild(element);
			
			element = doc.createElement("deal_name");
			element.appendChild(doc.createTextNode(dealName));
			serDeal.appendChild(element);
			
			element = doc.createElement("bal_info_index");
			element.appendChild(doc.createTextNode("0"));
			serDeal.appendChild(element);
			
			element = doc.createElement("subscription_index");
			element.appendChild(doc.createTextNode("0"));
			serDeal.appendChild(element);

			Element deal = doc.createElement("deal");
			rootElement.appendChild(deal);
			
			attr = doc.createAttribute("customization_flag");
			attr.setValue("optional");
			deal.setAttributeNode(attr);
	
			attr = doc.createAttribute("ondemand_billing");
			attr.setValue("no");
			deal.setAttributeNode(attr);
			
			
			element = doc.createElement("deal_name");
			element.appendChild(doc.createTextNode(dealName));
			deal.appendChild(element);
			
			element = doc.createElement("description");
			element.appendChild(doc.createTextNode(dealName));
			deal.appendChild(element);
			
			
			element = doc.createElement("permitted");
			element.appendChild(doc.createTextNode(service));
			deal.appendChild(element);
				
			if(planType.compareToIgnoreCase("RENTAL") != 0)
			{
				Element dealPrd1 = doc.createElement("deal_product");
				deal.appendChild(dealPrd1);
				
				attr = doc.createAttribute("status");
				attr.setValue("inactive");
				dealPrd1.setAttributeNode(attr);
				
				element = doc.createElement("product_name");
				element.appendChild(doc.createTextNode(depOutPrdName));
				dealPrd1.appendChild(element);
				element = doc.createElement("product_code");
				element.appendChild(doc.createTextNode(depOutPrdName));
				dealPrd1.appendChild(element);
				element = doc.createElement("quantity");
				element.appendChild(doc.createTextNode("1.0"));
				dealPrd1.appendChild(element);
	
				element = doc.createElement("purchase_discount");
				element.appendChild(doc.createTextNode("0.0"));
				dealPrd1.appendChild(element);
	
				element = doc.createElement("cycle_discount");
				element.appendChild(doc.createTextNode("0.0"));
				dealPrd1.appendChild(element);
			
				element = doc.createElement("usage_discount");
				element.appendChild(doc.createTextNode("0.0"));
				dealPrd1.appendChild(element);
		
				element = doc.createElement("status_flag");
				element.appendChild(doc.createTextNode("0"));
				dealPrd1.appendChild(element);
			}
			if (planType.compareToIgnoreCase("RENTAL-DEPOSIT") == 0 || planType.compareToIgnoreCase("RENTAL") == 0)
			{
				Element dealPrd1 = doc.createElement("deal_product");
				deal.appendChild(dealPrd1);
				
				attr = doc.createAttribute("status");
				attr.setValue("inactive");
				dealPrd1.setAttributeNode(attr);
				
				element = doc.createElement("product_name");
				element.appendChild(doc.createTextNode(rentPrdName));
				dealPrd1.appendChild(element);
				element = doc.createElement("product_code");
				element.appendChild(doc.createTextNode(rentPrdName));
				dealPrd1.appendChild(element);
				element = doc.createElement("quantity");
				element.appendChild(doc.createTextNode("1.0"));
				dealPrd1.appendChild(element);

				element = doc.createElement("purchase_discount");
				element.appendChild(doc.createTextNode("0.0"));
				dealPrd1.appendChild(element);

				element = doc.createElement("cycle_discount");
				element.appendChild(doc.createTextNode("0.0"));
				dealPrd1.appendChild(element);
			
				element = doc.createElement("usage_discount");
				element.appendChild(doc.createTextNode("0.0"));
				dealPrd1.appendChild(element);
		
				element = doc.createElement("status_flag");
				element.appendChild(doc.createTextNode("0"));
				dealPrd1.appendChild(element);
			}

			if(planType.compareToIgnoreCase("RENTAL") != 0)
			{
				Element PrdTag = doc.createElement("product");
				rootElement.appendChild(PrdTag);
				
				attr = doc.createAttribute("partial");
				attr.setValue("no");			
				PrdTag.setAttributeNode(attr);
				attr = doc.createAttribute("type");
				attr.setValue("subscription");			
				PrdTag.setAttributeNode(attr);
				
				element = doc.createElement("product_name");
				element.appendChild(doc.createTextNode(depOutPrdName));
				PrdTag.appendChild(element);
				
				element = doc.createElement("product_code");
				element.appendChild(doc.createTextNode(depOutPrdName));
				PrdTag.appendChild(element);
				
				element = doc.createElement("priority");
				element.appendChild(doc.createTextNode(depOutPriority));
				PrdTag.appendChild(element);
				
				element = doc.createElement("description");
				element.appendChild(doc.createTextNode(depOutPrdName));
				PrdTag.appendChild(element);
				
				element = doc.createElement("permitted");
				element.appendChild(doc.createTextNode(service));
				PrdTag.appendChild(element);
				
				Element eventRatMap = doc.createElement("event_rating_map");
				attr = doc.createAttribute("incr_unit");
				attr.setValue("none");			
				eventRatMap.setAttributeNode(attr);
				attr = doc.createAttribute("min_unit");
				attr.setValue("none");			
				eventRatMap.setAttributeNode(attr);
				attr = doc.createAttribute("rounding_rule");
				attr.setValue("nearest");			
				eventRatMap.setAttributeNode(attr);
				attr = doc.createAttribute("timezone_mode");
				attr.setValue("event");			
				eventRatMap.setAttributeNode(attr);
				attr = doc.createAttribute("tod_mode");
				attr.setValue("start_time");			
				eventRatMap.setAttributeNode(attr);
				PrdTag.appendChild(eventRatMap);
				
				
				element = doc.createElement("event_type");
				element.appendChild(doc.createTextNode(depOutEvent));
				eventRatMap.appendChild(element);
		
				element = doc.createElement("rum_name");
				element.appendChild(doc.createTextNode("Occurrence"));
				eventRatMap.appendChild(element);
				
				element = doc.createElement("min_quantity");
				element.appendChild(doc.createTextNode("1.0"));
				eventRatMap.appendChild(element);
				
				element = doc.createElement("incr_quantity");
				element.appendChild(doc.createTextNode("1.0"));
				eventRatMap.appendChild(element);
				
				element = doc.createElement("rate_plan_name");
				element.appendChild(doc.createTextNode("Product Purchase Fee Event"));
				eventRatMap.appendChild(element);
				
				Element ratePlan = doc.createElement("rate_plan");
				attr = doc.createAttribute("advance_billing_unit");
				attr.setValue("day");			
				ratePlan.setAttributeNode(attr);
				attr = doc.createAttribute("tax_when");
				attr.setValue("now");		
				ratePlan.setAttributeNode(attr);
				eventRatMap.appendChild(ratePlan);
				element = doc.createElement("rate_plan_name");
				element.appendChild(doc.createTextNode("Product Purchase Fee Event"));	
				ratePlan.appendChild(element);
				
				element = doc.createElement("currency_id");
				element.appendChild(doc.createTextNode(currency));
				ratePlan.appendChild(element);
				
				element = doc.createElement("event_type");
				element.appendChild(doc.createTextNode(depOutEvent));
				ratePlan.appendChild(element);
				
				element = doc.createElement("tax_code");
				element.appendChild(doc.createTextNode("MSO_VAT"));
				ratePlan.appendChild(element);
				
				element = doc.createElement("advance_billing_offset");
				element.appendChild(doc.createTextNode("0"));
				ratePlan.appendChild(element);
				
				element = doc.createElement("cycle_fee_flags");
				element.appendChild(doc.createTextNode("0"));
				ratePlan.appendChild(element);
				
				Element grantRateTier = doc.createElement("rate_tier");
				attr = doc.createAttribute("date_range_type");
				attr.setValue("absolute");			
				grantRateTier.setAttributeNode(attr);
				ratePlan.appendChild(grantRateTier);
				
				element = doc.createElement("rate_tier_name");
				element.appendChild(doc.createTextNode("Tier 1"));
				grantRateTier.appendChild(element);
				
				element = doc.createElement("priority");
				element.appendChild(doc.createTextNode("0.0"));
				grantRateTier.appendChild(element);
				
				
				Element rate = doc.createElement("rate");
	
				attr = doc.createAttribute("prorate_first");
				attr.setValue("prorate");			
				rate.setAttributeNode(attr);
				attr = doc.createAttribute("prorate_last");
				attr.setValue("full");			
				rate.setAttributeNode(attr);
	
				attr = doc.createAttribute("step_type");
				attr.setValue("total_quantity_rated");			
				rate.setAttributeNode(attr);
				attr = doc.createAttribute("type");
				attr.setValue("normal");			
				rate.setAttributeNode(attr);
				grantRateTier.appendChild(rate);
				
				element = doc.createElement("description");
				element.appendChild(doc.createTextNode("Rate 1"));
				rate.appendChild(element);
				
				element = doc.createElement("step_resource_id");
				element.appendChild(doc.createTextNode("0"));
				
				rate.appendChild(element);
				
				Element quantTier = doc.createElement("quantity_tier");
				rate.appendChild(quantTier);
				Element balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("flag");
				attr.setValue("discountable");			
			
				balImpact.setAttributeNode(attr);
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(currency));
				balImpact.appendChild(element);
				
				element = doc.createElement("impact_category");
				element.appendChild(doc.createTextNode("default"));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode(depOutGLId));
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode(depOutCharge));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				balImpact.appendChild(element);
			}
			if(planType.compareToIgnoreCase("RENTAL-DEPOSIT") == 0 || planType.compareToIgnoreCase("RENTAL") == 0)
			{
				Element PrdTag = doc.createElement("product");
				rootElement.appendChild(PrdTag);
				
				attr = doc.createAttribute("partial");
				attr.setValue("no");			
				PrdTag.setAttributeNode(attr);
				attr = doc.createAttribute("type");
				attr.setValue("subscription");			
				PrdTag.setAttributeNode(attr);
				
				element = doc.createElement("product_name");
				element.appendChild(doc.createTextNode(rentPrdName));
				PrdTag.appendChild(element);
				
				element = doc.createElement("product_code");
				element.appendChild(doc.createTextNode(rentPrdName));
				PrdTag.appendChild(element);
				
				element = doc.createElement("priority");
				element.appendChild(doc.createTextNode(rentPriority));
				PrdTag.appendChild(element);
				
				element = doc.createElement("description");
				element.appendChild(doc.createTextNode(rentPrdName));
				PrdTag.appendChild(element);
				
				element = doc.createElement("permitted");
				element.appendChild(doc.createTextNode(service));
				PrdTag.appendChild(element);
				
				
				Element eventRatMap = doc.createElement("event_rating_map");
				attr = doc.createAttribute("incr_unit");
				attr.setValue("none");			
				eventRatMap.setAttributeNode(attr);
				attr = doc.createAttribute("min_unit");
				attr.setValue("none");			
				eventRatMap.setAttributeNode(attr);
				attr = doc.createAttribute("rounding_rule");
				attr.setValue("nearest");			
				eventRatMap.setAttributeNode(attr);
				attr = doc.createAttribute("timezone_mode");
				attr.setValue("event");			
				eventRatMap.setAttributeNode(attr);
				attr = doc.createAttribute("tod_mode");
				attr.setValue("start_time");			
				eventRatMap.setAttributeNode(attr);
				PrdTag.appendChild(eventRatMap);
				
				
				element = doc.createElement("event_type");
				element.appendChild(doc.createTextNode(rentEvent));
				eventRatMap.appendChild(element);
		
				element = doc.createElement("rum_name");
				element.appendChild(doc.createTextNode("Occurrence"));
				eventRatMap.appendChild(element);
				
				element = doc.createElement("min_quantity");
				element.appendChild(doc.createTextNode("1.0"));
				eventRatMap.appendChild(element);
				
				element = doc.createElement("incr_quantity");
				element.appendChild(doc.createTextNode("1.0"));
				eventRatMap.appendChild(element);
				
				element = doc.createElement("rate_plan_name");
				element.appendChild(doc.createTextNode(planPayTerm+" Hardware "+planName));
				eventRatMap.appendChild(element);
				
				Element ratePlan = doc.createElement("rate_plan");
				attr = doc.createAttribute("advance_billing_unit");
				attr.setValue("day");			
				ratePlan.setAttributeNode(attr);
				attr = doc.createAttribute("tax_when");
				attr.setValue("now");		
				ratePlan.setAttributeNode(attr);
				eventRatMap.appendChild(ratePlan);
				element = doc.createElement("rate_plan_name");
				element.appendChild(doc.createTextNode(planPayTerm+" Hardware "+planName));	
				ratePlan.appendChild(element);
				
				element = doc.createElement("currency_id");
				element.appendChild(doc.createTextNode(currency));
				ratePlan.appendChild(element);
				
				element = doc.createElement("event_type");
				element.appendChild(doc.createTextNode(rentEvent));
				ratePlan.appendChild(element);
				
				element = doc.createElement("tax_code");
				element.appendChild(doc.createTextNode("MSO_Service_Tax"));
				ratePlan.appendChild(element);
				
				element = doc.createElement("advance_billing_offset");
				element.appendChild(doc.createTextNode("0"));
				ratePlan.appendChild(element);
				
				element = doc.createElement("cycle_fee_flags");
				element.appendChild(doc.createTextNode("0"));
				ratePlan.appendChild(element);
				
				Element rateTier = doc.createElement("rate_tier");
				attr = doc.createAttribute("date_range_type");
				attr.setValue("absolute");			
				rateTier.setAttributeNode(attr);
				ratePlan.appendChild(rateTier);
				
				element = doc.createElement("rate_tier_name");
				element.appendChild(doc.createTextNode("Tier 1"));
				rateTier.appendChild(element);
				
				element = doc.createElement("priority");
				element.appendChild(doc.createTextNode("0.0"));
				rateTier.appendChild(element);
				
				
				Element rate = doc.createElement("rate");

				attr = doc.createAttribute("prorate_first");
				attr.setValue("prorate");			
				rate.setAttributeNode(attr);
				attr = doc.createAttribute("prorate_last");
				attr.setValue("prorate");			
				rate.setAttributeNode(attr);

				attr = doc.createAttribute("step_type");
				attr.setValue("total_quantity_rated");			
				rate.setAttributeNode(attr);
				attr = doc.createAttribute("type");
				attr.setValue("normal");			
				rate.setAttributeNode(attr);
				rateTier.appendChild(rate);
				
				element = doc.createElement("description");
				element.appendChild(doc.createTextNode("Rate 1"));
				rate.appendChild(element);
				
				element = doc.createElement("step_resource_id");
				element.appendChild(doc.createTextNode("0"));
				
				rate.appendChild(element);
				
				Element quantTier = doc.createElement("quantity_tier");
				rate.appendChild(quantTier);
				Element balImpact = doc.createElement("balance_impact");
				attr = doc.createAttribute("flag");
				attr.setValue("discountable proratable");			
			
				balImpact.setAttributeNode(attr);
				attr = doc.createAttribute("scaled_unit");
				attr.setValue("none");			
				balImpact.setAttributeNode(attr);
				quantTier.appendChild(balImpact);
				
				element = doc.createElement("resource_id");
				element.appendChild(doc.createTextNode(currency));
				balImpact.appendChild(element);
				
				element = doc.createElement("glid");
				element.appendChild(doc.createTextNode(rentGLId));
				balImpact.appendChild(element);
				
				element = doc.createElement("fixed_amount");
				element.appendChild(doc.createTextNode("0.0"));
				balImpact.appendChild(element);
				
				element = doc.createElement("scaled_amount");
				element.appendChild(doc.createTextNode(rentCharge));
				balImpact.appendChild(element);
				balImpact.appendChild(element);
			}
			TransformerFactory transformerFactory = TransformerFactory.newInstance();
			Transformer transformer = transformerFactory.newTransformer();
			DOMSource source = new DOMSource(doc);
			//StreamResult result = new StreamResult(new File("D:\\CSV_File\\"+planName+".xml"));
			String fileName = planName.replace(" ", "_");
			StreamResult result = new StreamResult(new File(UtilOutDir+fileName+".xml"));
			transformer.transform(source, result);
	
			//logger.info("Pricing XML Generated!");
			logger.info("Pricing XML Generated!");
			return;
	    }
		catch (TransformerException tfe) 
		{
			logger.severe(tfe.getMessage());
			throw new IllegalArgumentException(tfe.getMessage());
			//tfe.printStackTrace();
		}
		catch (ParserConfigurationException pce) 
		{
			logger.severe(pce.getMessage());
			throw new IllegalArgumentException(pce.getMessage());
			//pce.printStackTrace();
		} 
		catch(Exception exception){
			logger.severe(exception.getMessage());
			throw new IllegalArgumentException(exception.getMessage());
	        //exception.printStackTrace();
		}
    }
    public static void dataValidation(Logger logger, String [] plan_data)
    {
	    /*int arrayLength = plan_data.length;
	    logger.info("Record Length: "+arrayLength);
	    if(arrayLength < 15)
	    {
	    	logger.info("Invalid Record Format");
	    	throw new IllegalArgumentException("Invalid Record Format");
	    }*/
    	//logger.info((Arrays.toString(plan_data)));
    	String serviceType = plan_data[0].trim();
	    String technology = plan_data[1].trim();
	    String payType = plan_data[2].trim();
	    String planType = plan_data[3].trim();
	    String planPayTerm = plan_data[4].trim();
	    String planName = plan_data[5].trim();
	    String provTag = plan_data[6].trim();
	    String cityWiseCharges = plan_data[7].trim();
	    String freeMBStr = plan_data[8].trim();
	    String chargePerMB =  plan_data[9].trim();
	    String chargeProrate = plan_data[10].trim();
	    String mbProrate = plan_data[11].trim();
	    String subPriority =  plan_data[12].trim();
	    String grantPriority =  plan_data[13].trim();
	    String usagePriority = "";
	    
        String semicolonSplitBy = ";";
        String colonSplitBy = ":";
        
	    if(planType != null && planType.compareToIgnoreCase("") != 0 
    			&& (planType.compareToIgnoreCase("UNLIMITED FUP") == 0
			    	|| planType.compareToIgnoreCase("LIMITED FUP") == 0
			    	|| planType.compareToIgnoreCase("UNLIMITED") == 0
			    	|| planType.compareToIgnoreCase("LIMITED") == 0))
	    {
	    	usagePriority = plan_data[14].trim();
    	}   
	    //logger.info("Inside Validation");
	    
	    int plan_count = planValidation(logger, planName, serviceType);
	    
	    if(plan_count > 0)
	    {
	    	logger.info("Plan already exists");
	    	throw new IllegalArgumentException("Plan already exists");
	    }
	    
	    
	    if(serviceType == null || serviceType.compareToIgnoreCase("") == 0 || serviceType.compareToIgnoreCase("BB") != 0)
	    {
	    	logger.info("Invalid Service Type");
	    	throw new IllegalArgumentException("Invalid Service Type");
	    }
	    
	    if(technology == null || technology.compareToIgnoreCase("") == 0)
	    {
	    	logger.info("Invalid Technology");
	    	throw new IllegalArgumentException("Invalid Technology");
	    }
	    
	    if(payType == null || payType.compareToIgnoreCase("") == 0 
	    		|| (payType.compareToIgnoreCase("POSTPAID") != 0 && payType.compareToIgnoreCase("PREPAID") != 0))
	    {
	    	logger.info("Invalid PayType");
	    	throw new IllegalArgumentException("Invalid PayType");
	    }
	    
	    if(planType == null || planType.compareToIgnoreCase("") == 0 
	    		|| (planType.compareToIgnoreCase("RENTAL-DEPOSIT") != 0 
	    				&& planType.compareToIgnoreCase("RENTAL") != 0
	    				&& planType.compareToIgnoreCase("OUTRIGHT") != 0
				    	&& planType.compareToIgnoreCase("UNLIMITED FUP") != 0
				    	&& planType.compareToIgnoreCase("LIMITED FUP") != 0
				    	&& planType.compareToIgnoreCase("UNLIMITED") != 0
				    	&& planType.compareToIgnoreCase("LIMITED") != 0
				    	&& planType.compareToIgnoreCase("AMC") != 0))
	    {
	    	logger.info("Invalid Plan Type");
	    	throw new IllegalArgumentException("Invalid Plan Type");
	    }
	    
	    if(planPayTerm == null || planPayTerm.compareToIgnoreCase("") == 0 
	    		|| (planPayTerm.compareToIgnoreCase("1") != 0 
	    				&& planPayTerm.compareToIgnoreCase("2") != 0
	    				&& planPayTerm.compareToIgnoreCase("3") != 0
				    	&& planPayTerm.compareToIgnoreCase("4") != 0
				    	&& planPayTerm.compareToIgnoreCase("6") != 0
				    	&& planPayTerm.compareToIgnoreCase("9") != 0
				    	&& planPayTerm.compareToIgnoreCase("12") != 0))
	    {
	    	logger.info("Invalid Plan Term");
	    	throw new IllegalArgumentException("Invalid Plan Term");
	    }
	    
	    if(planName == null || planName.compareToIgnoreCase("") == 0)
	    {
	    	logger.info("Invalid Plan Name");
	    	throw new IllegalArgumentException("Invalid Plan Name");
	    }
	    
	    if(planType != null && planType.compareToIgnoreCase("") != 0 
	    			&& (planType.compareToIgnoreCase("UNLIMITED FUP") == 0
				    	|| planType.compareToIgnoreCase("LIMITED FUP") == 0
				    	|| planType.compareToIgnoreCase("UNLIMITED") == 0
				    	|| planType.compareToIgnoreCase("LIMITED") == 0))
	    {
		    if(!(technology.compareToIgnoreCase("DOCSIS2") == 0 || 
		    		technology.compareToIgnoreCase("DOCSIS3") == 0 || 
		    		technology.compareToIgnoreCase("GPON") == 0 || 
		    		technology.compareToIgnoreCase("FIBER") == 0 || 
		    		technology.compareToIgnoreCase("ETHERNET") == 0))
		    {
		    	logger.info("Invalid technology");
		    	throw new IllegalArgumentException("Invalid technology");
		    }
		    
	    	if(cityWiseCharges == null || cityWiseCharges.compareToIgnoreCase("") == 0 || !cityWiseCharges.contains(":"))
	    	{
	    		logger.info("Invalid City Charges");
	    		throw new IllegalArgumentException("Invalid City Charges");
	    	}
	    	
			String[] citywisedata = cityWiseCharges.split(semicolonSplitBy, -1);
			String[] city = new String[100];

			int i=0;
			int arrayLength = citywisedata.length;
			while (i < arrayLength)
			{
				String[] citydata = citywisedata[i].trim().split(colonSplitBy, -1);
				city [i] = citydata [0].trim().toUpperCase();
			    int city_count = cityValidation(logger, city [i]);
			    if(city_count <= 0)
			    {
		    		logger.info("Invalid City "+city [i]);
			    	throw new IllegalArgumentException("Invalid City "+city [i]);
			    }
				i++;
			}
			
	    	if(provTag == null || provTag.compareToIgnoreCase("") == 0)
		    {
	    		logger.info("Invalid Prov Tag");
		    	throw new IllegalArgumentException("Invalid Prov Tag");
		    }
		    if (planType.compareToIgnoreCase("LIMITED FUP") == 0 && ( freeMBStr == null || !freeMBStr.contains(";")))
		    {
		    	logger.info("Invalid Free Quota");
		    	throw new IllegalArgumentException("Invalid Free Quota");
		    }
		    
		    if (planType.compareToIgnoreCase("LIMITED FUP") != 0 && ( freeMBStr == null || freeMBStr.compareToIgnoreCase("") == 0))
		    {
		    	logger.info("Invalid Free Quota");
		    	throw new IllegalArgumentException("Invalid Free Quota");
		    }
		    if (chargePerMB == null || chargePerMB.compareToIgnoreCase("") == 0)
		    {
		    	logger.info("Invalid Post Quota Charge");
		    	throw new IllegalArgumentException("Invalid Post Quota Charge");
		    }
		    if (chargeProrate == null || chargeProrate.compareToIgnoreCase("") == 0)
		    {
		    	logger.info("Invalid Charge Proration");
		    	throw new IllegalArgumentException("Invalid Charge Proration");
		    }
		    if (mbProrate == null || mbProrate.compareToIgnoreCase("") == 0)
		    {
		    	logger.info("Invalid Free Quota Proration");
		    	throw new IllegalArgumentException("Invalid Free Quota Proration");
		    }
		    if (subPriority == null || subPriority.compareToIgnoreCase("") == 0)
		    {
		    	logger.info("Invalid Subscription Product Priority");
		    	throw new IllegalArgumentException("Invalid Subscription Product Priority");
		    }
		    if (grantPriority == null || grantPriority.compareToIgnoreCase("") == 0)
		    {
		    	logger.info("Invalid Grant Product Priority");
		    	throw new IllegalArgumentException("Invalid Grant Product Priority");
		    }
		    if (usagePriority == null || usagePriority.compareToIgnoreCase("") == 0)
		    {
		    	logger.info("Invalid Usage Product Priorityn");
		    	throw new IllegalArgumentException("Invalid Usage Product Priority");
		    }
	    }
	    else if(planType != null && planType.compareToIgnoreCase("") != 0 
	    			&& (planType.compareToIgnoreCase("RENTAL-DEPOSIT") == 0 
	    				|| planType.compareToIgnoreCase("RENTAL") == 0
	    				|| planType.compareToIgnoreCase("OUTRIGHT") == 0))
	    {
		    if(!(technology.compareToIgnoreCase("Additional IP") == 0 || 
		    		technology.compareToIgnoreCase("CABLE MODEM") == 0 || 
		    		technology.compareToIgnoreCase("CABLE ROUTER") == 0 || 
		    		technology.compareToIgnoreCase("Cisco Soho Router") == 0 || 
		    		technology.compareToIgnoreCase("Huawei Router") == 0 || 
		    		technology.compareToIgnoreCase("Nat Device") == 0 || 
		    		technology.compareToIgnoreCase("ONU MODEM") == 0 || 
		    		technology.compareToIgnoreCase("ONU ROUTER") == 0 || 
		    		technology.compareToIgnoreCase("Wireless Nat Device") == 0 ||
		    		technology.compareToIgnoreCase("WIFI ROUTER") == 0 ||
		    		technology.compareToIgnoreCase("DOC3 CABLE MODEM") == 0))
		    {
		    	logger.info("Invalid technology");
		    	throw new IllegalArgumentException("Invalid technology");
		    }
	    	
	    	if (chargeProrate == null || chargeProrate.compareToIgnoreCase("") == 0)
		    {
		    	logger.info("Invalid Charge Proration");
		    	throw new IllegalArgumentException("Invalid Charge Proration");
		    }

		    if (planType.compareToIgnoreCase("RENTAL-DEPOSIT") == 0)
		    {
		    	if(grantPriority == null || grantPriority.compareToIgnoreCase("") == 0)
		    	{
		    		logger.info("Invalid DEPOSIT Product Priority");
		    		throw new IllegalArgumentException("Invalid DEPOSIT Product Priority");
		    	}
			    if (subPriority == null || subPriority.compareToIgnoreCase("") == 0)
			    {
			    	logger.info("Invalid RENTAL Product Priority");
			    	throw new IllegalArgumentException("Invalid RENTAL Product Priority");
			    }
		    	if(cityWiseCharges == null || cityWiseCharges.compareToIgnoreCase("") == 0 || !cityWiseCharges.contains(";"))
		    	{
		    		logger.info("Invalid Product Charges");
		    		throw new IllegalArgumentException("Invalid Product Charges");
		    	}
		    }
		    else if(planType.compareToIgnoreCase("RENTAL") == 0)
		    {
			    if (subPriority == null || subPriority.compareToIgnoreCase("") == 0 || subPriority.compareToIgnoreCase("0") == 0)
			    {
			    	logger.info("Invalid RENTAL Product Priority");
			    	throw new IllegalArgumentException("Invalid RENTAL Product Priority");
			    }
		    }
		    else if(planType.compareToIgnoreCase("OUTRIGHT") == 0)
		    {
			    if (grantPriority == null || grantPriority.compareToIgnoreCase("") == 0 || grantPriority.compareToIgnoreCase("0") == 0)
			    {
			    	logger.info("Invalid OUTRIGHT Product Priority");
			    	throw new IllegalArgumentException("Invalid OUTRIGHT Product Priority");
			    }
		    }
		    	
	    }
    }

    public static void catvDataValidation(Logger logger, String [] plan_data)
    {
	    String serviceType = plan_data[0].trim();
	    String planType = plan_data[1].trim();
	    String planSubType = plan_data[2].trim();
	    String payTerm = plan_data[3].trim();
	    String payUnit = plan_data[4].trim();
	    String planName = plan_data[5].trim();
	    String provTag = plan_data[6].trim();
	    String event = plan_data[7].trim();
	    String charge = plan_data[8].trim();
	    String priority = plan_data[9].trim();
	    String glid = plan_data[10].trim();
	    
	    int plan_count = planValidation(logger, planName, serviceType);
	    
	    if(plan_count > 0)
	    {
	    	logger.info("Plan already exists");
	    	throw new IllegalArgumentException("Plan already exists");
	    }
	   
	    
	    if(planType == null || planType.compareToIgnoreCase("") == 0)
	    {
	    	logger.info("Invalid Plan Type");
	    	throw new IllegalArgumentException("Invalid Plan Type");
	    }
	    
	    if(planSubType == null || planSubType.compareToIgnoreCase("") == 0)
	    {
	    	logger.info("Invalid Plan Sub Type");
	    	throw new IllegalArgumentException("Invalid Plan Sub Typ");
	    }
	    	    
	    if(provTag == null || provTag.compareToIgnoreCase("") == 0 )
	    {
	    	logger.info("Invalid Provisioning Tag");
	    	throw new IllegalArgumentException("Invalid Provisioning Tag");
	    }
	    
	    if(event == null || event.compareToIgnoreCase("") == 0 )
	    {
	    	logger.info("Invalid Event");
	    	throw new IllegalArgumentException("Invalid Event");
	    }
	    if(charge == null || charge.compareToIgnoreCase("") == 0 )
	    {
	    	logger.info("Invalid Charge");
	    	throw new IllegalArgumentException("Invalid Charge");
	    }
	    if(priority == null || priority.compareToIgnoreCase("") == 0 )
	    {
	    	logger.info("Invalid Priority");
	    	throw new IllegalArgumentException("Invalid Priority");
	    }
	    if(glid == null || glid.compareToIgnoreCase("") == 0 )
	    {
	    	logger.info("Invalid GLID");
	    	throw new IllegalArgumentException("Invalid GLID");
	    }
	    
	    if(payUnit == null || payUnit.compareToIgnoreCase("") == 0 
	    		|| (payUnit.compareToIgnoreCase("DAY") != 0 
	    				&& payUnit.compareToIgnoreCase("MONTH") != 0))
	    {
	    	logger.info("Invalid Pay Term Unit");
	    	throw new IllegalArgumentException("Invalid Pay Term Unit");
	    }
	    
	    if(payTerm == null || payTerm.compareToIgnoreCase("") == 0 )
	    {
	    	logger.info("Invalid Plan Term");
	    	throw new IllegalArgumentException("Invalid Plan Term");
	    }
	    
		if (payUnit.compareToIgnoreCase("MONTH") == 0)
    	{
			if(payTerm.compareToIgnoreCase("1") != 0 
					&& payTerm.compareToIgnoreCase("3") != 0
			    	&& payTerm.compareToIgnoreCase("6") != 0
			    	&& payTerm.compareToIgnoreCase("9") != 0
			    	&& payTerm.compareToIgnoreCase("12") != 0)
			{
				logger.info("Invalid Month Plan Term");
				throw new IllegalArgumentException("Invalid Plan Term");
			}
    	}
		else if (payUnit.compareToIgnoreCase("DAY") == 0 && Integer.parseInt(payTerm) < 1)
		{
	    	logger.info("Invalid Day Plan Term");
	    	throw new IllegalArgumentException("Invalid Plan Term");
    	}
		
	    if(planName == null || planName.compareToIgnoreCase("") == 0)
	    {
	    	logger.info("Invalid Plan Name");
	    	throw new IllegalArgumentException("Invalid Plan Name");
	    }
    }
    
    public static void createCATVPlan (Logger logger, String [] plan_data, String description)
    {                
        try
    	{
    	    String serviceType = plan_data[0].trim();
    	    String planType = plan_data[1].trim();
    	    String payType = plan_data[2].trim();
    	    String payTerm = plan_data[3].trim();
    	    String payTermUnit = plan_data[4].trim();
    	    String planName = plan_data[5].trim();
    	    String provTag = plan_data[6].trim();
    	    String event = plan_data[7].trim();
    	    String charge = plan_data[8].trim();
    	    String priority = plan_data[9].trim();
    	    String glid = plan_data[10].trim();
            
            String dealName = planName;
            String prdName = planName;
    		String service = "/service/catv";
    		

    		String currency = "356";
    		
    		DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
    		DocumentBuilder docBuilder = docFactory.newDocumentBuilder();

    		// root elements
    		Document doc = docBuilder.newDocument();
    		
    		Element rootElement = doc.createElement("price_list");
    		doc.appendChild(rootElement);
    		
    		Attr attr = doc.createAttribute("version");
    		attr.setValue("7.2");
    		rootElement.setAttributeNode(attr);
    		
    		attr = doc.createAttribute("xmlns:xsi");
    		attr.setValue("http://www.w3.org/2001/XMLSchema-instance");
    		rootElement.setAttributeNode(attr);
    		
    		attr = doc.createAttribute("xsi:noNamespaceSchemaLocation");
    		attr.setValue("price_list.xsd");
    		rootElement.setAttributeNode(attr);
    		Element plan = doc.createElement("plan");
    		rootElement.appendChild(plan);
    		
    		attr = doc.createAttribute("ondemand_billing");
    		attr.setValue("no");
    		plan.setAttributeNode(attr);

    		Element element = doc.createElement("plan_name");
    		element.appendChild(doc.createTextNode(planName));
    		plan.appendChild(element);
    		
    		element = doc.createElement("description");
    		element.appendChild(doc.createTextNode(description));
    		plan.appendChild(element);
    		
    		Element serDeal = doc.createElement("service_deal");
    		plan.appendChild(serDeal);
    		
    		element = doc.createElement("service_name");
    		element.appendChild(doc.createTextNode(service));
    		serDeal.appendChild(element);
    		
    		element = doc.createElement("deal_name");
    		element.appendChild(doc.createTextNode(dealName));
    		serDeal.appendChild(element);
    		
    		element = doc.createElement("bal_info_index");
    		element.appendChild(doc.createTextNode("0"));
    		serDeal.appendChild(element);
    		
    		element = doc.createElement("subscription_index");
    		element.appendChild(doc.createTextNode("0"));
    		serDeal.appendChild(element);

    		Element deal = doc.createElement("deal");
    		rootElement.appendChild(deal);
    		
    		attr = doc.createAttribute("customization_flag");
    		attr.setValue("optional");
    		deal.setAttributeNode(attr);

    		attr = doc.createAttribute("ondemand_billing");
    		attr.setValue("no");
    		deal.setAttributeNode(attr);
    		
    		
    		element = doc.createElement("deal_name");
    		element.appendChild(doc.createTextNode(dealName));
    		deal.appendChild(element);
    		
    		element = doc.createElement("description");
    		element.appendChild(doc.createTextNode(description));
    		deal.appendChild(element);
    		
    		
    		element = doc.createElement("permitted");
    		element.appendChild(doc.createTextNode(service));
    		deal.appendChild(element);
    			
			Element dealPrd1 = doc.createElement("deal_product");
			deal.appendChild(dealPrd1);
			
			attr = doc.createAttribute("status");
			attr.setValue("inactive");
			dealPrd1.setAttributeNode(attr);
			
			element = doc.createElement("product_name");
			element.appendChild(doc.createTextNode(prdName));
			dealPrd1.appendChild(element);
			element = doc.createElement("product_code");
			element.appendChild(doc.createTextNode(prdName));
			dealPrd1.appendChild(element);
			element = doc.createElement("quantity");
			element.appendChild(doc.createTextNode("1.0"));
			dealPrd1.appendChild(element);

			element = doc.createElement("purchase_end");
			attr = doc.createAttribute("unit");
			attr.setValue(payTermUnit.toLowerCase());
			element.setAttributeNode(attr);
			element.appendChild(doc.createTextNode(payTerm));
			dealPrd1.appendChild(element);
			
			element = doc.createElement("purchase_discount");
			element.appendChild(doc.createTextNode("0.0"));
			dealPrd1.appendChild(element);
			
			element = doc.createElement("cycle_end");
			attr = doc.createAttribute("unit");
			attr.setValue(payTermUnit.toLowerCase());
			element.setAttributeNode(attr);
			element.appendChild(doc.createTextNode(payTerm));
			dealPrd1.appendChild(element);

			element = doc.createElement("cycle_discount");
			element.appendChild(doc.createTextNode("0.0"));
			dealPrd1.appendChild(element);
			
			element = doc.createElement("usage_end");
			attr = doc.createAttribute("unit");
			attr.setValue(payTermUnit.toLowerCase());
			element.setAttributeNode(attr);
			element.appendChild(doc.createTextNode(payTerm));
			dealPrd1.appendChild(element);

			element = doc.createElement("status_flag");
			element.appendChild(doc.createTextNode("0"));
			dealPrd1.appendChild(element);
	
			Element PrdTag = doc.createElement("product");
			rootElement.appendChild(PrdTag);
			
			attr = doc.createAttribute("partial");
			attr.setValue("no");			
			PrdTag.setAttributeNode(attr);
			attr = doc.createAttribute("type");
			attr.setValue("subscription");			
			PrdTag.setAttributeNode(attr);
			
			element = doc.createElement("product_name");
			element.appendChild(doc.createTextNode(prdName));
			PrdTag.appendChild(element);
			
			element = doc.createElement("product_code");
			element.appendChild(doc.createTextNode(prdName));
			PrdTag.appendChild(element);
			
			element = doc.createElement("priority");
			element.appendChild(doc.createTextNode(priority));
			PrdTag.appendChild(element);
			
			element = doc.createElement("description");
			element.appendChild(doc.createTextNode(description));
			PrdTag.appendChild(element);
			
			element = doc.createElement("permitted");
			element.appendChild(doc.createTextNode(service));
			PrdTag.appendChild(element);
			
			element = doc.createElement("provisioning");
			element.appendChild(doc.createTextNode(provTag));
			PrdTag.appendChild(element);

			Element eventRatMap = doc.createElement("event_rating_map");
			attr = doc.createAttribute("incr_unit");
			attr.setValue("none");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("min_unit");
			attr.setValue("none");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("rounding_rule");
			attr.setValue("nearest");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("timezone_mode");
			attr.setValue("event");			
			eventRatMap.setAttributeNode(attr);
			attr = doc.createAttribute("tod_mode");
			attr.setValue("start_time");			
			eventRatMap.setAttributeNode(attr);
			PrdTag.appendChild(eventRatMap);
			
			
			element = doc.createElement("event_type");
			element.appendChild(doc.createTextNode(event));
			eventRatMap.appendChild(element);
	
			element = doc.createElement("rum_name");
			element.appendChild(doc.createTextNode("Occurrence"));
			eventRatMap.appendChild(element);
			
			element = doc.createElement("min_quantity");
			element.appendChild(doc.createTextNode("1.0"));
			eventRatMap.appendChild(element);
			
			element = doc.createElement("incr_quantity");
			element.appendChild(doc.createTextNode("1.0"));
			eventRatMap.appendChild(element);
			
			element = doc.createElement("rate_plan_name"); 
		        if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_thirty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Thirty Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_thirty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Thirty Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_thirty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Thirty Days Cycle Forward Ala-Carte fixed duration"));
			}
		        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_sixty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Sixty Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_sixty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Sixty Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_sixty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Sixty Days Cycle Forward Ala-Carte fixed duration"));
			}
		        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_ninety_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Ninety Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_ninety_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Ninety Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_ninety_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Ninety Days Cycle Forward Ala-Carte fixed duration"));
			}
		        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_oneeighty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Eighty Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_oneeighty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Eighty Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_oneeighty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Eighty Days Cycle Forward Ala-Carte fixed duration"));
			}
			// Jyothirmayi Kachiraju == Changes for new FDP event creation
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_onetwenty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Twenty Days Cycle Forward Base fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 120 days base package event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_onetwenty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Twenty Days Cycle Forward Addon fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 120 days add on event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_onetwenty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Twenty Days Cycle Forward Ala-Carte fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 120 days ala-carte event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_onefifty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Hundred and Fifty Days Cycle Forward Base fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 150 days base package event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_onefifty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Hundred and Fifty Days Cycle Forward Addon fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 150 days add on event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_onefifty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Hundred and Fifty Days Cycle Forward Ala-Carte fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 150 days ala-carte event >> "+event);
			}
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_three_days/mso_sb_pkg_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Three Days Cycle Forward Base fixed duration"));
                                logger.info("PricingUtility:createCATVPlan === 3 days base package event >> "+event);
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_three_days/mso_sb_adn_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Three Days Cycle Forward Addon fixed duration"));
                                logger.info("PricingUtility:createCATVPlan === 3 days add on event >> "+event);
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_three_days/mso_sb_alc_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Three Days Cycle Forward Ala-Carte fixed duration"));
                                logger.info("PricingUtility:createCATVPlan === 3 days ala-carte event >> "+event);
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_weekly/mso_sb_pkg_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Weekly Cycle Forward Base fixed duration"));
                                logger.info("PricingUtility:createCATVPlan === Weekly base package event >> "+event);
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_weekly/mso_sb_adn_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Weekly Cycle Forward Addon fixed duration"));
                                logger.info("PricingUtility:createCATVPlan === Weekly add on event >> "+event);
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_weekly/mso_sb_alc_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Weekly Cycle Forward Ala-Carte fixed duration"));
                                logger.info("PricingUtility:createCATVPlan === Weekly ala-carte event >> "+event);
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_fifteen_days/mso_sb_pkg_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Fifteen Days Cycle Forward Base fixed duration"));
                                logger.info("PricingUtility:createCATVPlan === 15 Days base package event >> "+event);
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_fifteen_days/mso_sb_adn_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Fifteen Days Cycle Forward Addon fixed duration"));
                                logger.info("PricingUtility:createCATVPlan === 15 Days add on event >> "+event);
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_fifteen_days/mso_sb_alc_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Fifteen Days Cycle Forward Ala-Carte fixed duration"));
                                logger.info("PricingUtility:createCATVPlan === 15 Days ala-carte event >> "+event);
                        }
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoten_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Ten Days Cycle Forward Base fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 210 days base package event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoten_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Ten Days Cycle Forward Addon fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 210 days add on event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoten_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Ten Days Cycle Forward Ala-Carte fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 210 days ala-carte event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoforty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Forty Days Cycle Forward Base fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 240 days base package event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoforty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Forty Days Cycle Forward Addon fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 240 days add on event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoforty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Forty Days Cycle Forward Ala-Carte fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 240 days ala_carte event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoseventy_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Seventy Days Cycle Forward Base fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 270 days base package event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoseventy_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Seventy Days Cycle Forward Addon fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 270 days add on event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoseventy_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Seventy Days Cycle Forward Ala-Carte fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 270 days ala_carte event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threehundred_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred Days Cycle Forward Base fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 300 days base package event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threehundred_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred Days Cycle Forward Addon fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 300 days add on event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threehundred_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred Days Cycle Forward Ala-Carte fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 300 days ala-carte event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threethirty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred and Thirty Days Cycle Forward Base fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 330 days base package event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threethirty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred and Thirty Days Cycle Forward Addon fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 330 days add on event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threethirty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred and Thirty Days Cycle Forward Ala-Carte fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 330 days ala-carte event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threesixty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred and Sixty Days Cycle Forward Base fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 360 days base package event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threesixty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred and Sixty Days Cycle Forward Addon fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 360 days add on event >> "+event);
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threesixty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred and Sixty Days Cycle Forward Ala-Carte fixed duration"));
				logger.info("PricingUtility:createCATVPlan === 360 days ala-carte event >> "+event);
			}
			else
			{
				element.appendChild(doc.createTextNode("Monthly Cycle Forward Ala-Carte "+payType));
				logger.info("PricingUtility:createCATVPlan === Not Days Event  >> "+event);
			}
			eventRatMap.appendChild(element);
			
			Element ratePlan = doc.createElement("rate_plan");
			attr = doc.createAttribute("advance_billing_unit");
			attr.setValue("day");			
			ratePlan.setAttributeNode(attr);
			attr = doc.createAttribute("tax_when");
			attr.setValue("now");		
			ratePlan.setAttributeNode(attr);
			eventRatMap.appendChild(ratePlan);
			element = doc.createElement("rate_plan_name");
		        if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_thirty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Thirty Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_thirty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Thirty Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_thirty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Thirty Days Cycle Forward Ala-Carte fixed duration"));
			}
		        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_sixty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Sixty Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_sixty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Sixty Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_sixty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Sixty Days Cycle Forward Ala-Carte fixed duration"));
			}
		        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_ninety_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Ninety Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_ninety_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Ninety Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_ninety_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Ninety Days Cycle Forward Ala-Carte fixed duration"));
			}
		        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_oneeighty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Eighty Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_oneeighty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Eighty Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_oneeighty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Eighty Days Cycle Forward Ala-Carte fixed duration"));
			}
						else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_onetwenty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Twenty Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_onetwenty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Twenty Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_onetwenty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Twenty Days Cycle Forward Ala-Carte fixed duration"));
			}
			// Jyothirmayi Kachiraju == Changes for new FDP event creation
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_onefifty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Hundred and Fifty Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_onefifty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Hundred and Fifty Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_onefifty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("One Hundred and Fifty Days Cycle Forward Ala-Carte fixed duration"));
			}
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_three_days/mso_sb_pkg_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Three Days Cycle Forward Base fixed duration"));
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_three_days/mso_sb_adn_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Three Days Cycle Forward Addon fixed duration"));
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_three_days/mso_sb_alc_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Three Days Cycle Forward Ala-Carte fixed duration"));
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_weekly/mso_sb_pkg_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Weekly Cycle Forward Base fixed duration"));
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_weekly/mso_sb_adn_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Weekly Cycle Forward Addon fixed duration"));
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_weekly/mso_sb_alc_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Weekly Cycle Forward Ala-Carte fixed duration"));
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_fifteen_days/mso_sb_pkg_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Fifteen Days Cycle Forward Base fixed duration"));
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_fifteen_days/mso_sb_adn_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Fifteen Days Cycle Forward Addon fixed duration"));
                        }
                        else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_fifteen_days/mso_sb_alc_fdp") == 0)
                        {
                                element.appendChild(doc.createTextNode("Fifteen Days Cycle Forward Ala-Carte fixed duration"));
                        }
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoten_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Ten Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoten_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Ten Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoten_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Ten Days Cycle Forward Ala-Carte fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoforty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Forty Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoforty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Forty Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoforty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Forty Days Cycle Forward Ala-Carte fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoseventy_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Seventy Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoseventy_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Seventy Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_twoseventy_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Two Hundred and Seventy Days Cycle Forward Ala-Carte fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threehundred_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threehundred_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threehundred_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred Days Cycle Forward Ala-Carte fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threethirty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred and Thirty Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threethirty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred and Thirty Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threethirty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred and Thirty Days Cycle Forward Ala-Carte fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threesixty_days/mso_sb_pkg_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred and Sixty Days Cycle Forward Base fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threesixty_days/mso_sb_adn_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred and Sixty Days Cycle Forward Addon fixed duration"));
			}
			else if (event.compareToIgnoreCase("/event/billing/product/fee/cycle/cycle_forward_threesixty_days/mso_sb_alc_fdp") == 0)
			{
				element.appendChild(doc.createTextNode("Three Hundred and Sixty Days Cycle Forward Ala-Carte fixed duration"));
			}
			else
			{
				element.appendChild(doc.createTextNode("Monthly Cycle Forward Ala-Carte "+payType));
			}
			ratePlan.appendChild(element);
			
			element = doc.createElement("currency_id");
			element.appendChild(doc.createTextNode(currency));
			ratePlan.appendChild(element);
			
			element = doc.createElement("event_type");
			element.appendChild(doc.createTextNode(event));
			ratePlan.appendChild(element);
			
			// Jyothirmayi Kachiraju == Changes for new FDP event creation			
			logger.info("PricingUtility:createCATVPlan === Tax_Code Node creation >> "+event);
			
			element = doc.createElement("tax_code");
		    if (event.contains("cycle_forward_thirty_days") || event.contains("cycle_forward_sixty_days") ||
				event.contains("cycle_forward_ninety_days") || event.contains("cycle_forward_onetwenty_days") ||
				event.contains("cycle_forward_oneeighty_days") || event.contains("cycle_forward_onefifty_days") || 
				event.contains("cycle_forward_weekly") || event.contains("cycle_forward_three_days") ||
				event.contains("cycle_forward_twoten_days") || event.contains("cycle_forward_twoforty_days") || 
				event.contains("cycle_forward_twoseventy_days") || event.contains("cycle_forward_threehundred_days") ||
				event.contains("cycle_forward_threethirty_days") || event.contains("cycle_forward_threesixty_days") || 
				event.contains("cycle_forward_fifteen_days"))
			{
				element.appendChild(doc.createTextNode("9984"));
				logger.info("PricingUtility:createCATVPlan === days event >> "+event);
			}
			else
			{
				element.appendChild(doc.createTextNode("MSO_Service_Tax"));
				logger.info("PricingUtility:createCATVPlan === days event >> "+event);
			}
			ratePlan.appendChild(element);
			
			element = doc.createElement("advance_billing_offset");
			element.appendChild(doc.createTextNode("0"));
			ratePlan.appendChild(element);
			
			element = doc.createElement("cycle_fee_flags");
			element.appendChild(doc.createTextNode("0"));
			ratePlan.appendChild(element);
			
			Element rateTier = doc.createElement("rate_tier");
			attr = doc.createAttribute("date_range_type");
			attr.setValue("absolute");			
			rateTier.setAttributeNode(attr);
			ratePlan.appendChild(rateTier);
			
			element = doc.createElement("rate_tier_name");
			element.appendChild(doc.createTextNode("Tier 1"));
			rateTier.appendChild(element);
			
			element = doc.createElement("priority");
			element.appendChild(doc.createTextNode("0.0"));
			rateTier.appendChild(element);
			
			
			Element rate = doc.createElement("rate");
			
			attr = doc.createAttribute("prorate_first");
			attr.setValue("full");			
			rate.setAttributeNode(attr);
			attr = doc.createAttribute("prorate_last");
		        //if (event.contains("cycle_forward_thirty_days"))
			//{
			//	attr.setValue("full");			
			//}
			//else
			//{
				attr.setValue("prorate");			
			//}
			rate.setAttributeNode(attr);

			attr = doc.createAttribute("step_type");
			attr.setValue("total_quantity_rated");			
			rate.setAttributeNode(attr);
			attr = doc.createAttribute("type");
			attr.setValue("normal");			
			rate.setAttributeNode(attr);
			rateTier.appendChild(rate);
			
			element = doc.createElement("description");
			element.appendChild(doc.createTextNode("Rate 1"));
			rate.appendChild(element);
			
			element = doc.createElement("step_resource_id");
			element.appendChild(doc.createTextNode("0"));
			
			rate.appendChild(element);
			
			Element quantTier = doc.createElement("quantity_tier");
			rate.appendChild(quantTier);
			Element balImpact = doc.createElement("balance_impact");
			attr = doc.createAttribute("flag");
		        //if (event.contains("cycle_forward_thirty_days"))
			//{
			//	attr.setValue("discountable");			
			//}
			//else
			//{
				attr.setValue("discountable proratable");			
			//}
		
			balImpact.setAttributeNode(attr);
			attr = doc.createAttribute("scaled_unit");
			attr.setValue("none");			
			balImpact.setAttributeNode(attr);
			quantTier.appendChild(balImpact);
			
			element = doc.createElement("resource_id");
			element.appendChild(doc.createTextNode(currency));
			balImpact.appendChild(element);
			
			element = doc.createElement("glid");
			element.appendChild(doc.createTextNode(glid));
			balImpact.appendChild(element);
			
			element = doc.createElement("fixed_amount");
			element.appendChild(doc.createTextNode(charge));
			balImpact.appendChild(element);
			
			element = doc.createElement("scaled_amount");
			element.appendChild(doc.createTextNode("0.0"));
			balImpact.appendChild(element);
			element = doc.createElement("relative_end_offset");
			attr = doc.createAttribute("unit");
			attr.setValue(payTermUnit.toLowerCase());			
			element.setAttributeNode(attr);
			element.appendChild(doc.createTextNode(payTerm));
			
    		TransformerFactory transformerFactory = TransformerFactory.newInstance();
    		Transformer transformer = transformerFactory.newTransformer();
    		DOMSource source = new DOMSource(doc);
    		//StreamResult result = new StreamResult(new File("D:\\CSV_File\\"+planName+".xml"));
    		String fileName = planName.replace(" ", "_");
    		StreamResult result = new StreamResult(new File(UtilOutDir+fileName+".xml"));
    		transformer.transform(source, result);

    		//logger.info("Pricing XML Generated!");
    		logger.info("Pricing XML Generated!");
    		return;
        }
    	catch (TransformerException tfe) 
    	{
    		logger.severe(tfe.getMessage());
    		throw new IllegalArgumentException(tfe.getMessage());
    		//tfe.printStackTrace();
    	}
    	catch (ParserConfigurationException pce) 
    	{
    		logger.severe(pce.getMessage());
    		throw new IllegalArgumentException(pce.getMessage());
    		//pce.printStackTrace();
    	} 
    	catch(Exception exception){
    		logger.severe(exception.getMessage());
    		throw new IllegalArgumentException(exception.getMessage());
            //exception.printStackTrace();
    	}
    }

    public static void insertCATVPricingData(Logger logger, String [] plan_data, String description)
	{
		Connection mainConnection = null;
	    Statement statement = null;
	    ResultSet resultSet = null;
		try{
		    Class.forName("oracle.jdbc.driver.OracleDriver");
			mainConnection=DriverManager.getConnection(ConnectStr, User, Pass);
			
			String insertStatement= "insert into pricing_catv_master_t values (plan_id_catv.NEXTVAL,'"+plan_data[0].trim()+"','"+plan_data[1].trim()+"','"+
			plan_data[2].trim()+"','"+plan_data[3].trim()+"','"+plan_data[4].trim()+"','"+plan_data[5].trim()+"','"+plan_data[6].trim()+"','"+
			plan_data[7].trim()+"','"+plan_data[8].trim()+"','"+plan_data[9].trim()+"','"+plan_data[10].trim()+"','"+description+"','0')";
			
			//	logger.info(insertStatement);
			statement = mainConnection.createStatement();
			statement.execute(insertStatement);		
		}
		catch (Exception e)
		{
			//logger.info("Here go custom msg");
			//logger.info("Error : " + e.getMessage());
			logger.severe(e.getMessage());
	        throw new IllegalArgumentException("Error inserting data in Table");
		}
	    finally
	    {
	    	try
            {
                statement.close();

                if (mainConnection != null)
                {
                	mainConnection.close();
                }
            }
            catch (Exception ignored)
            {}
	    }
	}
}

