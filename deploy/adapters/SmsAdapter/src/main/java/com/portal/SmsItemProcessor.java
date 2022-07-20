package com.portal;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLEncoder;
import java.util.Properties;

import javax.mail.Message;
import javax.mail.Session;
import javax.mail.Transport;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeMessage;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.log4j.Logger;
import org.springframework.batch.item.ItemProcessor;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;



public class SmsItemProcessor implements ItemProcessor<Sms, Sms> {
	
	private static final Logger logger = Logger.getLogger(SmsItemProcessor.class);
	
	private String smsSuccessMsg;
	private String smsServerName;
	private String smsUsernameTag;
	private String smsUsernameVal;
	private String smsPasswordTag;
	private String smsPasswordVal;
	private String smsFeedIdTag;
	private String smsFeedIdVal;
	private String smsUserDefParam;
	private String smsMobile;
	private String smsMessage;
	private boolean smsMailAlertReqd;
	private String smsMailFrom;
	private String smsMailTo;
	private String smsMailSubject;
	private String smsMailText;
	private String smsMailSmtpHost;
	
	
	public String getSmsSuccessMsg() {
		return smsSuccessMsg;
	}

	public void setSmsSuccessMsg(String smsSuccessMsg) {
		this.smsSuccessMsg = smsSuccessMsg;
	}

	public String getSmsServerName() {
		return smsServerName;
	}

	public void setSmsServerName(String smsServerName) {
		this.smsServerName = smsServerName;
	}

	public String getSmsUsernameTag() {
		return smsUsernameTag;
	}

	public void setSmsUsernameTag(String smsUsernameTag) {
		this.smsUsernameTag = smsUsernameTag;
	}

	public String getSmsUsernameVal() {
		return smsUsernameVal;
	}

	public void setSmsUsernameVal(String smsUsernameVal) {
		this.smsUsernameVal = smsUsernameVal;
	}

	public String getSmsPasswordTag() {
		return smsPasswordTag;
	}

	public void setSmsPasswordTag(String smsPasswordTag) {
		this.smsPasswordTag = smsPasswordTag;
	}

	public String getSmsPasswordVal() {
		return smsPasswordVal;
	}

	public void setSmsPasswordVal(String smsPasswordVal) {
		this.smsPasswordVal = smsPasswordVal;
	}

	public String getSmsFeedIdTag() {
		return smsFeedIdTag;
	}

	public void setSmsFeedIdTag(String smsFeedIdTag) {
		this.smsFeedIdTag = smsFeedIdTag;
	}

	public String getSmsFeedIdVal() {
		return smsFeedIdVal;
	}

	public void setSmsFeedIdVal(String smsFeedIdVal) {
		this.smsFeedIdVal = smsFeedIdVal;
	}

	public String getSmsUserDefParam() {
		return smsUserDefParam;
	}

	public void setSmsUserDefParam(String smsUserDefParam) {
		this.smsUserDefParam = smsUserDefParam;
	}

	public String getSmsMobile() {
		return smsMobile;
	}

	public void setSmsMobile(String smsMobile) {
		this.smsMobile = smsMobile;
	}

	public String getSmsMessage() {
		return smsMessage;
	}

	public void setSmsMessage(String smsMessage) {
		this.smsMessage = smsMessage;
	}

	public boolean isSmsMailAlertReqd() {
		return smsMailAlertReqd;
	}

	public void setSmsMailAlertReqd(boolean smsMailAlertReqd) {
		this.smsMailAlertReqd = smsMailAlertReqd;
	}

	public String getSmsMailFrom() {
		return smsMailFrom;
	}

	public void setSmsMailFrom(String smsMailFrom) {
		this.smsMailFrom = smsMailFrom;
	}

	public String getSmsMailTo() {
		return smsMailTo;
	}

	public void setSmsMailTo(String smsMailTo) {
		this.smsMailTo = smsMailTo;
	}

	public String getSmsMailSubject() {
		return smsMailSubject;
	}

	public void setSmsMailSubject(String smsMailSubject) {
		this.smsMailSubject = smsMailSubject;
	}

	public String getSmsMailText() {
		return smsMailText;
	}

	public void setSmsMailText(String smsMailText) {
		this.smsMailText = smsMailText;
	}

	public String getSmsMailSmtpHost() {
		return smsMailSmtpHost;
	}

	public void setSmsMailSmtpHost(String smsMailSmtpHost) {
		this.smsMailSmtpHost = smsMailSmtpHost;
	}

	@Override
	public Sms process(Sms item) throws Exception {
		int status = 1;
	    HttpURLConnection httpURLConnection = null;
		URL url = null;
		URLConnection urlConnection = null;
		BufferedReader reader = null;

		String response;
		String resp = "";
		boolean sMailReqd = smsMailAlertReqd;
		String sSuccessMsg = smsSuccessMsg;
		String sSMSServerName = smsServerName;
		String sSMSUsernameTag = smsUsernameTag;
		String sSMSUsernameValue = smsUsernameVal;
		String sSMSPasswordTag = smsPasswordTag;
		String sSMSPasswordValue = smsPasswordVal;
		String sSMSFeedidTag = smsFeedIdTag;
		String sSMSFeedidValue = smsFeedIdVal;
		String sSMSUserDefParam = smsUserDefParam;
		String sSMSMobileParam = smsMobile;
		String sSMSMessageParam = smsMessage;
		
		String strServer;
		String strMobileNoParameter;
		String strMessageParameter;
	    logger.debug("Processing..." + item.getNtf_id());

	    try {
	    	strServer = sSMSServerName + "?";
			if ((!(sSMSUsernameTag.equalsIgnoreCase(""))) && (!(sSMSUsernameValue.equalsIgnoreCase("")))) {
			  strServer = strServer + sSMSUsernameTag + "=" + sSMSUsernameValue + "&";
			}
			if ((!(sSMSPasswordTag.equalsIgnoreCase(""))) && (!(sSMSPasswordValue.equalsIgnoreCase("")))) {
			  strServer = strServer + sSMSPasswordTag + "=" + sSMSPasswordValue + "&";
			}
			if ((!(sSMSFeedidTag.equalsIgnoreCase(""))) && (!(sSMSFeedidValue.equalsIgnoreCase("")))) {
			  strServer = strServer + sSMSFeedidTag + "=" + sSMSFeedidValue + "&";
			} else {
				System.out.println("Feed Id is a mandatory field to be configured.");
				throw new Exception("Missing required configuration - Feed Id.");
			}

			if ((!(sSMSMobileParam.equalsIgnoreCase("")))) {
			  strMobileNoParameter = sSMSMobileParam;
			} else {
			  strMobileNoParameter = "PhoneNumber";
			}

			if ((!(sSMSMessageParam.equalsIgnoreCase("")))) {
			  strMessageParameter = sSMSMessageParam;
			} else {
			  strMessageParameter = "text";
			}

			String msgText = URLEncoder.encode(item.getMessage(), "UTF-8");
            String urlAddress = strServer + strMobileNoParameter + "=" + item.getMsisdn() + "&" + strMessageParameter + "=" + msgText + "&" + sSMSUserDefParam;
            logger.debug("URL Prepared : " + urlAddress);
            try {
                url = new URL(urlAddress);
            } catch(MalformedURLException e) {
            	logger.error("URL Connection " + e.getClass().getName() + " Occurred.", e);
                throw new MalformedURLException("URL Connection " + e.getClass().getName() + " Occurred.");
            } 
            try {
                urlConnection = url.openConnection();
                httpURLConnection = (HttpURLConnection)urlConnection;
            } catch(IOException e) {
            	logger.error("Http Client " + e.getClass().getName() + " Occurred.", e);
                throw new IOException("Http Client " + e.getClass().getName() + " Occurred.");               
            }
            try {
                reader = new BufferedReader(new InputStreamReader(httpURLConnection.getInputStream()));
            } catch(IOException e) {
            	logger.error("Client's reader " + e.getClass().getName() + " Occurred.", e);
                throw new IOException("Client's reader " + e.getClass().getName() + " Occurred.");               
            }
       
            while((response = reader.readLine()) != null) 
                resp = resp + response;
	    	logger.info("Response received : " + resp);
            if(resp.length() > sSuccessMsg.length()) {
                if(resp.toUpperCase().contains(sSuccessMsg.toUpperCase())) {
                    if(resp.toUpperCase().contains("ERROR")) {
                    	status = 2;
                    	readResponse(item, resp);
                        logger.error("Failed to sent the message for ID " + item.getMsisdn() + " ,Reason : " + resp);
                    } else {
                    	status = 0;
                    }
                } else {
                    status = 2;
                    readResponse(item, resp);
                    logger.error("Failed to sent the message for ID " + item.getMsisdn() + " ,Reason : " + resp);
                }
            } else {
                status = 2;
                readResponse(item, resp);
                logger.error("Failed to sent the message for ID " + item.getMsisdn() + " ,Reason : " + resp);
            }
	    } catch(Exception e) {
	    	logger.error("Exception Occurred in sendSMS : ", e);
	        status = 2;
	    } finally {
	        try {
	            if(reader != null) reader.close();
	            if(httpURLConnection != null) httpURLConnection = null;
	            if(urlConnection != null) urlConnection = null;
	            if(url != null) url = null;
	        } catch(IOException e) {
	            logger.error("Exception Occurred in sendSMS while closing server connection objects : ", e);
	        }
	    } 
	    if (sMailReqd)
	    	recordTimeForAlerting(item);
	    item.setStatus(status);
	    item.setModT(System.currentTimeMillis()/1000);
	    logger.debug("Job Item Status: " + item.getStatus());
	    logger.debug("Response: " + item.getErr_code() + " : " + item.getErr_desc());
		return item;
	}
	
	public void readResponse(Sms item, String response) throws SAXException, ParserConfigurationException, IOException {
	        Document doc = stringToDom(response);
	    	doc.getDocumentElement().normalize();
	    	item.setErr_code(new Integer(doc.getElementsByTagName("CODE").item(0).getTextContent()).intValue());
	    	item.setErr_desc(doc.getElementsByTagName("DESC").item(0).getTextContent());
	    	logger.debug("Response Error Code " + item.getErr_code());
	    	logger.debug("Response Error Description " + item.getErr_desc());
	}
	
	public boolean recordTimeForAlerting(Sms item) {
		String sSMSMailFrom = smsMailFrom;
		String sSMSMailTo = smsMailTo;
		String sSMSMailSubject = smsMailSubject;
		String sSMSMailText = smsMailText;
		String sSMSMailSmtpAdd = smsMailSmtpHost;
		
		Properties mailProp;
		Session sess;
		Message msg;

		try {
	        mailProp = System.getProperties();
	        mailProp.put("mail.smtp.host", sSMSMailSmtpAdd);
	        sess = Session.getDefaultInstance(mailProp, null);
	        msg = new MimeMessage(sess);
	        msg.setFrom(new InternetAddress(sSMSMailFrom));
	        msg.setRecipients(javax.mail.Message.RecipientType.TO, InternetAddress.parse(sSMSMailTo, false));
	        msg.setSubject(sSMSMailSubject);
	        msg.setText(sSMSMailText);
            Transport.send(msg);
            logger.debug("MAIL SENT SUCCESSFULLY");
	        return true;
		} catch(Exception e) {
	        logger.error("Exception at recordTimeForAlerting : ", e);
	    }
	    return false;
	}
	
	public Document stringToDom(String xmlSource) 
            throws SAXException, ParserConfigurationException, IOException {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        DocumentBuilder builder = factory.newDocumentBuilder();
        return builder.parse(new InputSource(new StringReader(xmlSource)));
    }
	
}
