import java.sql.*;
import java.io.*;
import java.util.*;
import java.util.logging.*;

import java.nio.charset.Charset;
import java.nio.charset.CharsetEncoder;


class CMTSConfig
{
	  private String ipPoolValue = "";
	  private String talValue = "";
	  private String dhcpValue = "";
	  
	  public String getIpPoolValue() {
	    return ipPoolValue;
	  }

	  public void setIpPoolValue(String ipPoolValue) {
	    this.ipPoolValue = ipPoolValue;
	  }

	  public String getTalValue() {
	    return talValue;
	  }

	  public void setTalValue(String talValue) {
	    this.talValue = talValue;
	  }

	  public String getDhcpValue() {
		    return dhcpValue;
	  }
	  
	  public void setDhcpValue(String dhcpValue ) {
	    this.dhcpValue = dhcpValue;
	  }
}

class CMTSData
{
	
	  private long cmtsPdp;
	  private int recId;


	  public long getcmtsPdp() {
	    return cmtsPdp;
	  }

	  public void setcmtsPdp(long cmtsPdp) {
	    this.cmtsPdp = cmtsPdp;
	  }

	  public int getrecId() {
	    return recId;
	  }

	  public void setrecId(int recId) {
	    this.recId = recId;
	  }
}

public class ProvCmtsMap {

	public static String ConnectStr = "jdbc:oracle:thin:@172.20.20.23:1723:pindb";
	public static String User = "pin";
	public static String Pass = "UgypCdDf";
	
	public static String UtilOutDir = "/home/pin/opt/portal/7.5/mso/apps/mso_pricing_utility/output/";
	public static String UtilLogDir = "/home/pin/opt/portal/7.5/mso/apps/mso_pricing_utility/logs/";
	public static String UtilInDir = "/home/pin/opt/portal/7.5/mso/apps/mso_pricing_utility/";

	//public static String UtilOutDir = "D:\\CSV_File\\";
	//public static String UtilLogDir = "D:\\CSV_File\\";
	//public static String UtilInDir = "D:\\CSV_File\\";
	public static int	transactionStatus;		// 1 for open, 2 for abort, 3 for commit

	static CharsetEncoder asciiEncoder = 
	      Charset.forName("US-ASCII").newEncoder(); // or "ISO-8859-1" for ISO Latin 1

	public static boolean isPureAscii(String v) {
	    return asciiEncoder.canEncode(v);
	}

	public static void main(String[] args)
    {
    	String inputFileName = args[0];
    	//String inputFileName = "prov_inp.csv";
        Logger logger = Logger.getLogger("ProvCmtsMapLog");  
        FileHandler fh; 
    	
    	String csvFile = UtilInDir+inputFileName;
        BufferedReader br = null;
        String line = "";
        String cvsSplitBy = ",";

        try
        {
            fh = new FileHandler(UtilLogDir+inputFileName+".log");  
            logger.addHandler(fh);
            SimpleFormatter formatter = new SimpleFormatter();  
            fh.setFormatter(formatter); 
            logger.info("Start Processing File: "+inputFileName);
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
		                String[] prov_data = line.trim().split(cvsSplitBy, -1);
		                //logger.info((Arrays.toString(plan_data)));
	    		    	//logger.info("Calling Validation");
		                logger.info("Loading Record: ");
		                logger.info((Arrays.toString(prov_data)));
		                logger.info("Validating Data");
	    		    	dataValidation(logger, prov_data);
	    		    	cmtsProvMapping(logger, prov_data);
		                successFile.println(line);
		                //insert data in cmts prov master table
		                insertCmtsProvData(logger, prov_data);
	            	}
	    		    catch (Exception e)
	    		    {
	    		    	logger.severe("Failed to Process Record");
	    		    	errorFile.println(line+","+e.getMessage());
	    		    	logger.severe(e.getMessage());
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
    }

    public static void cmtsProvMapping (Logger logger, String [] prov_data)
    {
		Connection mainConnection = null;
	    ResultSet resultSet = null;
	    Statement statement = null;
	    Statement statement1 = null;
        try
		{        	
		    String provTag = prov_data[0].trim();
		    String technology = prov_data[1].trim().toUpperCase();
		    String servCode = prov_data[2].trim();
		    String planType = prov_data[3].trim().toUpperCase();
		    String cities = prov_data[4].trim();
		   
		    /*int	technologyInt = 0;

		    if (technology.toUpperCase().compareToIgnoreCase("DOCSIS2") == 0)
		    {
		    	technologyInt =  1;
		    }
		    else if (technology.toUpperCase().compareToIgnoreCase("DOCSIS3") == 0)
		    {
		    	technologyInt =  2;
		    }
		    else if (technology.toUpperCase().compareToIgnoreCase("ETHERNET") == 0)
		    {
		    	technologyInt =  3;
		    }
		    else if (technology.toUpperCase().compareToIgnoreCase("FIBER") == 0)
		    {
		    	technologyInt =  4;
		    }
		    else if (technology.toUpperCase().compareToIgnoreCase("GPON") == 0)
		    {
		    	technologyInt =  5;
		    }*/
		    
		    String[] cityArray = cities.split(";");
		    int i = 0;
		    while ( i < cityArray.length)
		    {
			    Class.forName("oracle.jdbc.driver.OracleDriver");
				mainConnection=DriverManager.getConnection("jdbc:oracle:thin:@172.20.20.23:1723:pindb", "pin", "UgypCdDf");
				//mainConnection.setAutoCommit(false);
				//String getProv= "select cmts_id, city from MSO_CFG_CMTS_MASTER_T where city = '"+cityArray[i]+"' and technology = "+technologyInt;
				String getProv = "";
				if (technology.compareToIgnoreCase("GPON") == 0)
				{	
					//getProv= "select distinct cmts_id from MSO_CFG_CMTS_MASTER_T where city = '"+cityArray[i]+"' and technology = 5";
					getProv= "select distinct cmts_id from MSO_CFG_CMTS_MASTER_T where city = '"+cityArray[i]+
							"' and technology = 5  and cmts_id not in (select m1.cmts_id from MSO_CFG_CMTS_CC_MAPPING_T m1, "+
							"MSO_CC_MAPPING_T m2 where m1.poid_id0=m2.obj_id0 and m2.provisioning_tag = '"+provTag+"')";
				}
				else if (technology.compareToIgnoreCase("FIBER") == 0)
				{	
					//getProv= "select distinct cmts_id from MSO_CFG_CMTS_MASTER_T where city = '"+cityArray[i]+"' and technology = 4";
					getProv= "select distinct cmts_id from MSO_CFG_CMTS_MASTER_T where city = '"+cityArray[i]+
							"' and technology = 4  and cmts_id not in (select m1.cmts_id from MSO_CFG_CMTS_CC_MAPPING_T m1, "+
							"MSO_CC_MAPPING_T m2 where m1.poid_id0=m2.obj_id0 and m2.provisioning_tag = '"+provTag+"')";
				}
				else if (technology.compareToIgnoreCase("ETHERNET") == 0)
				{	
					//getProv= "select distinct cmts_id from MSO_CFG_CMTS_MASTER_T where city = '"+cityArray[i]+"' and technology = 3";
					getProv= "select distinct cmts_id from MSO_CFG_CMTS_MASTER_T where city = '"+cityArray[i]+
							"' and technology = 3  and cmts_id not in (select m1.cmts_id from MSO_CFG_CMTS_CC_MAPPING_T m1, "+
							"MSO_CC_MAPPING_T m2 where m1.poid_id0=m2.obj_id0 and m2.provisioning_tag = '"+provTag+"')";
				}
				else
				{	
					//getProv= "select distinct cmts_id from MSO_CFG_CMTS_MASTER_T where city = '"+cityArray[i]+"' and technology in (1,2)";
					getProv= "select distinct cmts_id from MSO_CFG_CMTS_MASTER_T where city = '"+cityArray[i]+
							"' and technology in (1,2)  and cmts_id not in (select m1.cmts_id from MSO_CFG_CMTS_CC_MAPPING_T m1, "+
							"MSO_CC_MAPPING_T m2 where m1.poid_id0=m2.obj_id0 and m2.provisioning_tag = '"+provTag+"')";
				}
				//logger.info(getProv);
				Class.forName("oracle.jdbc.driver.OracleDriver");
				statement = mainConnection.createStatement();
				statement1 = mainConnection.createStatement();
				resultSet = statement.executeQuery(getProv);
				while(resultSet.next())
				{
					String cmts = resultSet.getString(1);
					CMTSConfig pc = new CMTSConfig();
					pc = getCmtsConfig(logger, cmts, technology, planType);
					String ipPoolValue = pc.getIpPoolValue();
					String talValue = pc.getTalValue();
					String dhcpValue = pc.getDhcpValue();
					
					if ((ipPoolValue == null || ipPoolValue.compareToIgnoreCase("") == 0) &&
							(talValue == null || talValue.compareToIgnoreCase("") == 0) &&
							(dhcpValue == null || dhcpValue.compareToIgnoreCase("") == 0))
					{
						logger.info("Config not found for CMTS: "+cmts+"skipping..");
						continue;
					}
					int maxRec = 0;
					long cmtsPdp = 0;
					try{
						CMTSData cd = getCmtsPoid(logger, cmts);
						maxRec = cd.getrecId();
						cmtsPdp = cd.getcmtsPdp();
					}
					catch (Exception e)
					{
						continue;
					}
					if (technology.compareToIgnoreCase("GPON") == 0 || technology.compareToIgnoreCase("FIBER") == 0 || technology.compareToIgnoreCase("ETHERNET") == 0)
					{
						String insertStatement= "insert into MSO_CC_MAPPING_T values ("+cmtsPdp+","+(maxRec+1)+",100,'IP Pool-Static','"+
						ipPoolValue+"','"+provTag+"','"+servCode+"')";
						
						//logger.info(insertStatement);
						//statement = mainConnection.createStatement();
						statement1.execute(insertStatement);
					}
					else if (technology.compareToIgnoreCase("DOCSIS3") == 0 || 
							technology.compareToIgnoreCase("DOCSIS2") == 0 ||
							technology.compareToIgnoreCase("GOLD") == 0 ||
							technology.compareToIgnoreCase("NB") == 0 )
					{
						String insertStatement = "";
						if (ipPoolValue != null && ipPoolValue.compareToIgnoreCase("") != 0)
						{
							insertStatement= "insert into MSO_CC_MAPPING_T values ("+cmtsPdp+","+(maxRec+1)+",100,'IP Pool-Static','"+
							ipPoolValue+"','"+provTag+"','"+servCode+"')";
							
							//logger.info(insertStatement);
							//statement = mainConnection.createStatement();
							statement1.execute(insertStatement);
						}
						if (talValue != null && talValue.compareToIgnoreCase("") != 0)
						{
							insertStatement= "insert into MSO_CC_MAPPING_T values ("+cmtsPdp+","+(maxRec+2)+",200,'TAL CL','"+
							talValue+"','"+provTag+"','"+servCode+"')";
							
							//logger.info(insertStatement);
							statement1.execute(insertStatement);
						}
						if (dhcpValue != null && dhcpValue.compareToIgnoreCase("") != 0)
						{
							insertStatement= "insert into MSO_CC_MAPPING_T values ("+cmtsPdp+","+(maxRec+3)+",300,'DHCP CL','"+
							dhcpValue+"','"+provTag+"','"+servCode+"')";
							
							//logger.info(insertStatement);
							statement1.execute(insertStatement);
						}
					}
				}
				i++;
		    }
		    //mainConnection.commit();
		 	logger.info("Configuration Done");
			return;
	    }
		catch(Exception exception){
	    	try
            {
	    		//mainConnection.rollback();
            }
            catch (Exception ignored)
            {}
            throw new IllegalArgumentException(exception.getMessage());
	        //exception.printStackTrace();
		}
	    finally
	    {
	    	try
            {
                resultSet.close();
                statement.close();
                statement1.close();
                if (mainConnection != null)
                {
                	mainConnection.close();
                }
            }
            catch (Exception ignored)
            {}
	    }
    }

	public static CMTSConfig getCmtsConfig(Logger logger, String cmts, String technology, String planType)
	{
		Connection mainConnection = null;
	    ResultSet resultSet = null;
	    Statement statement = null;
	    CMTSConfig cc = new CMTSConfig();
		try{
		    Class.forName("oracle.jdbc.driver.OracleDriver");
			mainConnection=DriverManager.getConnection("jdbc:oracle:thin:@172.20.20.23:1723:pindb", "pin", "UgypCdDf");
			
			String getCMTSConfig = "";
			if(technology.compareToIgnoreCase("DOCSIS3") == 0)
			{	
				getCMTSConfig= "select doc3_tal, doc3_dhcp, doc3_ip_pool from cmts_config_mapping_t where upper(cmts_id) = '"+cmts.toUpperCase()+"'";
			}
			else if(technology.compareToIgnoreCase("DOCSIS2") == 0 && planType.compareToIgnoreCase("LIMITED") == 0)
			{	
				getCMTSConfig= "select doc2_tal, doc2_dhcp_ltd, doc2_ip_pool_ltd from cmts_config_mapping_t where upper(cmts_id) = '"+cmts.toUpperCase()+"'";
			}
			else if(technology.compareToIgnoreCase("DOCSIS2") == 0 && planType.compareToIgnoreCase("UNLIMITED") == 0)
			{	
				getCMTSConfig= "select doc2_tal, doc2_dhcp_unltd, doc2_ip_pool_unltd from cmts_config_mapping_t where upper(cmts_id) = '"+cmts.toUpperCase()+"'";
			}
			else if(technology.compareToIgnoreCase("GOLD") == 0 && planType.compareToIgnoreCase("LIMITED") == 0)
			{	
				getCMTSConfig= "select gold_tal, gold_dhcp_ltd, gold_ip_pool from cmts_config_mapping_t where upper(cmts_id) = '"+cmts.toUpperCase()+"'";
			}
			else if(technology.compareToIgnoreCase("GOLD") == 0 && planType.compareToIgnoreCase("UNLIMITED") == 0)
			{	
				getCMTSConfig= "select gold_tal, gold_dhcp_unltd, gold_ip_pool from cmts_config_mapping_t where upper(cmts_id) = '"+cmts.toUpperCase()+"'";
			}
			else if(technology.compareToIgnoreCase("NB") == 0)
			{	
				getCMTSConfig= "select nb_tal, nb_dhcp, nb_ip_pool from cmts_config_mapping_t where upper(cmts_id) = '"+cmts.toUpperCase()+"'";
			}
			else if(technology.compareToIgnoreCase("FIBER") == 0)
			{	
				getCMTSConfig= "select fiber_ip_pool from cmts_config_mapping_t where upper(cmts_id) = '"+cmts.toUpperCase()+"'";
			}
			else if(technology.compareToIgnoreCase("GPON") == 0 || technology.compareToIgnoreCase("ETHERNET") == 0)
			{	
				getCMTSConfig= "select gpon_ip_pool from cmts_config_mapping_t where upper(cmts_id) = '"+cmts.toUpperCase()+"'";
			}
			
			//logger.info(getCMTSConfig);
			logger.info("Fetching Configuration..");
			Class.forName("oracle.jdbc.driver.OracleDriver");
			statement = mainConnection.createStatement();
			resultSet = statement.executeQuery(getCMTSConfig);
			while(resultSet.next())
			{
				if (technology.compareToIgnoreCase("FIBER") != 0 && technology.compareToIgnoreCase("GPON") != 0 &&
						technology.compareToIgnoreCase("ETHERNET") != 0)
				{
					cc.setTalValue(resultSet.getString(1));
					cc.setDhcpValue(resultSet.getString(2));
					cc.setIpPoolValue(resultSet.getString(3));
				}
				
				if (technology.compareToIgnoreCase("FIBER") == 0 || technology.compareToIgnoreCase("GPON") == 0 ||
						technology.compareToIgnoreCase("ETHERNET") == 0)
				{
					cc.setIpPoolValue(resultSet.getString(1));
				}
			}
		}
		catch (Exception e)
		{
			//logger.info("Here go custom msg");
			//logger.info("Error : " + e.getMessage());
			logger.info("Config Not Found for CMTS: "+cmts+" Skipping");
	        //throw new IllegalArgumentException("Config not found for combination");
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
	    return cc;
	}
	
	public static CMTSData getCmtsPoid(Logger logger, String cmts)
	{
		Connection mainConnection = null;
	    ResultSet resultSet = null;
	    Statement statement = null;
	    CMTSData cd = new CMTSData();
		try{
		    Class.forName("oracle.jdbc.driver.OracleDriver");
			mainConnection=DriverManager.getConnection("jdbc:oracle:thin:@172.20.20.23:1723:pindb", "pin", "UgypCdDf");
			
			String getCmtsPoid= "select poid_id0 from MSO_CFG_CMTS_CC_MAPPING_T where upper(cmts_id) = '"+cmts.toUpperCase()+"'";
			//logger.info(getPricingConfig);
			logger.info("Fetching CMTS POID..");
			Class.forName("oracle.jdbc.driver.OracleDriver");
			statement = mainConnection.createStatement();
			resultSet = statement.executeQuery(getCmtsPoid);
			int maxRec = 0;
			if(resultSet.next())
			{
					long cmtsPdp = resultSet.getLong(1);
					cd.setcmtsPdp(cmtsPdp);
					String getMaxRec= "select max(rec_id) from MSO_CC_MAPPING_T where obj_id0 = "+cmtsPdp;
					statement = mainConnection.createStatement();
					resultSet = statement.executeQuery(getMaxRec);
					if(resultSet.next())
					{
							maxRec = resultSet.getInt(1);
							cd.setrecId(maxRec);
					}
					else
					{
						maxRec = 0;
					}
			}
			else
			{
				//logger.info("Configuration not found");
				logger.warning("CMTS not Found");
				throw new IllegalArgumentException("CMTS not found in mapping:"+cmts);
			}
		}
		catch (Exception e)
		{
			//logger.info("Here go custom msg");
			//logger.info("Error : " + e.getMessage());
			logger.warning(e.getMessage());
	        throw new IllegalArgumentException("CMTS not found in Mapping:"+cmts);
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
	    return cd;
	}
	public static void insertCmtsProvData(Logger logger, String [] prov_data)
	{
		Connection mainConnection = null;
	    Statement statement = null;
	    ResultSet resultSet = null;
		try
		{
		    Class.forName("oracle.jdbc.driver.OracleDriver");
			mainConnection=DriverManager.getConnection(ConnectStr, User, Pass);
			
			String insertStatement= "insert into cmts_master_data_t values (cmts_seq.NEXTVAL,'"+prov_data[0].trim()+"','"+prov_data[1].trim()+"','"+
			prov_data[2].trim()+"','"+prov_data[3].trim()+"','"+prov_data[4].trim()+"')";
			
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
	
	public static int provValidation(Logger logger, String provTag)
	{
		Connection mainConnection = null;
	    ResultSet resultSet = null;
	    Statement statement = null;
	    int prov_count = -1;
		try{
		    Class.forName("oracle.jdbc.driver.OracleDriver");
			mainConnection=DriverManager.getConnection(ConnectStr, User, Pass);
			
			String getProv= "select count(*) from mso_config_bb_pt_t where provisioning_tag = '"+provTag+"'";
			logger.info("Validating Provisioning Tag..");
			Class.forName("oracle.jdbc.driver.OracleDriver");
			statement = mainConnection.createStatement();
			resultSet = statement.executeQuery(getProv);
			if(resultSet.next())
			{
					prov_count = resultSet.getInt(1);
					return prov_count;
			}
			else
			{
				logger.severe("Error in searching Provisionign Tag in Config");
				throw new IllegalArgumentException("Error in searching Provisioning Tag in Config");
			}
		}
		catch (Exception e)
		{
			logger.severe(e.getMessage());
	        throw new IllegalArgumentException("Error in searching Provisioning Tag");
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
	public static int cityValidation(Logger logger, String city)
	{
		Connection mainConnection = null;
	    ResultSet resultSet = null;
	    Statement statement = null;
	    int isAvailable = -1;
		try
		{
		    Class.forName("oracle.jdbc.driver.OracleDriver");
			mainConnection=DriverManager.getConnection(ConnectStr, User, Pass);
			
			String getProv= "select count(*) from MSO_CFG_CMTS_MASTER_T where city = '"+city+"'";
			logger.info("Validating City..");
			Class.forName("oracle.jdbc.driver.OracleDriver");
			statement = mainConnection.createStatement();
			resultSet = statement.executeQuery(getProv);
			if(resultSet.next())
			{
				isAvailable = resultSet.getInt(1);
				return isAvailable;
			}
			else
			{
				logger.severe("Error in Validating City Config");
				throw new IllegalArgumentException("Error in Validating City Config");
			}
		}
		catch (Exception e)
		{
			logger.severe(e.getMessage());
	        throw new IllegalArgumentException("Error in Validating City");
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
    public static void dataValidation(Logger logger, String [] prov_data)
    {
    	String provTag = prov_data[0].trim();
	    String technology = prov_data[1].trim();
	    String serviceCode = prov_data[2].trim();
	    String planType = prov_data[3].trim();
	    String cities = prov_data[4].trim();
	    
	    logger.info("Validating Data");
	    
	    int prov_count = provValidation(logger, provTag);
	    
	    if(prov_count == 0)
	    {
	    	logger.info("Provisioning tag Doesn't exist");
	    	throw new IllegalArgumentException("Provisioning tag Doesn't exist");
	    }
	    
	    if(technology == null || technology.compareToIgnoreCase("") == 0)
	    {
	    	logger.info("Invalid Technology");
	    	throw new IllegalArgumentException("Invalid Technology");
	    }
	    else if(technology.compareToIgnoreCase("DOCSIS2") != 0 && 
	    		technology.compareToIgnoreCase("DOCSIS3") != 0 &&
	    		technology.compareToIgnoreCase("GPON") != 0 &&
	    		technology.compareToIgnoreCase("ETHERNET") != 0 &&
	    		technology.compareToIgnoreCase("FIBER") != 0)
	    {
	    	logger.info("Invalid Technology");
	    	throw new IllegalArgumentException("Invalid Technology");
	    }
	    
	    if(serviceCode == null || serviceCode.compareToIgnoreCase("") == 0)
	    {
	    	logger.info("Invalid Service Code");
	    	throw new IllegalArgumentException("Invalid Service Code");
	    }
	    
	    if(planType == null || planType.compareToIgnoreCase("") == 0)
	    {
	    	logger.info("Invalid Plan Type");
	    	throw new IllegalArgumentException("Invalid Plan Type");
	    }
	    else if(planType.compareToIgnoreCase("LIMITED") != 0 && 
	    		planType.compareToIgnoreCase("UNLIMITED") != 0)
	    {
	    	logger.info("Invalid Plan Type");
	    	throw new IllegalArgumentException("Invalid Plan Type");
	    }
	    
	    if(cities == null || cities.compareToIgnoreCase("") == 0)
	    {
	    	logger.info("Invalid cities");
	    	throw new IllegalArgumentException("Invalid cities");
	    }
	    
	    String[] cityArray = cities.split(";");
	    int i = 0;
	    while ( i < cityArray.length)
	    {
	    	int isAvailable = cityValidation(logger, cityArray[i]);
	    	if(isAvailable <= 0)
	    	{
	    		logger.info("Invalid city: "+cityArray[i]);
		    	throw new IllegalArgumentException("Invalid city: "+cityArray[i]);
	    	}
	    	else
	    	{
	    		logger.info("Valid City: "+cityArray[i]);
	    	}
	    	i++;
	    }
    }

}
