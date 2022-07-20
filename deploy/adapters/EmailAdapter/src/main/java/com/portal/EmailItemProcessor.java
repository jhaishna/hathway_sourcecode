package com.portal;

import java.util.Properties;

import javax.mail.Message;
import javax.mail.MessagingException;
import javax.mail.SendFailedException;
import javax.mail.Session;
import javax.mail.Transport;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeMessage;

import org.apache.log4j.Logger;
import org.springframework.batch.item.ItemProcessor;

import com.sun.mail.smtp.SMTPAddressFailedException;
import com.sun.mail.smtp.SMTPAddressSucceededException;
import com.sun.mail.smtp.SMTPSendFailedException;
import com.sun.mail.smtp.SMTPTransport;


public class EmailItemProcessor implements ItemProcessor<Email, Email> {
	
	private static final Logger logger = Logger.getLogger(EmailItemProcessor.class);
	
	private String smtpHost;
	private int smtpPort;
	private boolean smtpAuth;
	private String smtpUsername;
	private String smtpPassword;
	
	public String getSmtpHost() {
		return smtpHost;
	}

	public void setSmtpHost(String smtpHost) {
		this.smtpHost = smtpHost;
	}

	public int getSmtpPort() {
		return smtpPort;
	}

	public void setSmtpPort(int smtpPort) {
		this.smtpPort = smtpPort;
	}

	public boolean isSmtpAuth() {
		return smtpAuth;
	}

	public void setSmtpAuth(boolean smtpAuth) {
		this.smtpAuth = smtpAuth;
	}

	public String getSmtpUsername() {
		return smtpUsername;
	}

	public void setSmtpUsername(String smtpUsername) {
		this.smtpUsername = smtpUsername;
	}

	public String getSmtpPassword() {
		return smtpPassword;
	}

	public void setSmtpPassword(String smtpPassword) {
		this.smtpPassword = smtpPassword;
	}



	@Override
	public Email process(Email item) throws Exception {
		int status = 1;
	    boolean verbose = false;
	    logger.debug("Processing..." + item.getNtf_id());

	    try {
	        Properties props = System.getProperties();
	        props.put("mail.smtp.host", smtpHost);
	        props.put("mail.smtp.port", smtpPort);
				
			Session session = Session.getDefaultInstance(props, null);
	   
	        Message message = new MimeMessage(session);
			message.setFrom(new InternetAddress(item.getEmailFrom()));
			message.setRecipients(javax.mail.Message.RecipientType.TO, InternetAddress.parse(item.getEmailTo(), false));
			message.setSubject(item.getSubject());
			message.setText(item.getMessage());
	        
			logger.debug("Host: " + smtpHost + " Port: " + smtpPort);
	        SMTPTransport t = (SMTPTransport)session.getTransport("smtp"); 
	        try {
	        	if (smtpAuth) {
	        		t.connect(smtpHost, smtpPort, smtpUsername, smtpPassword);
	        	} else {
	        		t.connect();
	        	}
	        	t.sendMessage(message,message.getAllRecipients()); 
	        } finally {
	            t.close();
	            item.setErr_code(t.getLastReturnCode());
	            item.setErr_desc(t.getLastServerResponse());
	            status = 0;
	        }
	    } catch(Exception e) {
		    // Handle SMTP-specific exceptions.
	    	if (e instanceof SendFailedException) {
	    		MessagingException sfe = (MessagingException)e;
	    		if (sfe instanceof SMTPSendFailedException) {
	    			SMTPSendFailedException ssfe = (SMTPSendFailedException)sfe;
	    			logger.debug("SMTP SEND FAILED:");
	    			if (verbose)
	    				logger.debug(ssfe.toString());
	    			item.setErr_code(ssfe.getReturnCode());
		            item.setErr_desc(ssfe.getMessage());
	    		} else {
	    			if (verbose)
	    				logger.debug("Send failed: " + sfe.toString());
	    		}
	    		Exception ne;
	    		while ((ne = sfe.getNextException()) != null &&
	    				ne instanceof MessagingException) {
	    			sfe = (MessagingException)ne;
	    			if (sfe instanceof SMTPAddressFailedException) {
	    				SMTPAddressFailedException ssfe =
	    						(SMTPAddressFailedException)sfe;
	    				logger.debug("ADDRESS FAILED:");
	    				if (verbose)
	    					logger.debug(ssfe.toString());
	    				item.setErr_code(ssfe.getReturnCode());
			            item.setErr_desc(ssfe.getMessage());
	    			} else if (sfe instanceof SMTPAddressSucceededException) {
	    				logger.debug("ADDRESS SUCCEEDED:");
	    				SMTPAddressSucceededException ssfe =
	    						(SMTPAddressSucceededException)sfe;
	    				if (verbose)
	    				item.setErr_code(ssfe.getReturnCode());
			            item.setErr_desc(ssfe.getMessage());
	    			}
	    		}
	    	} 
	    	status = 2;
	    } 
	    item.setStatus(status);
	    item.setModT(System.currentTimeMillis()/1000);
	    logger.debug("Job Item Status: " + item.getStatus());
	    logger.debug("Response: " + item.getErr_code() + " : " + item.getErr_desc());
		return item;
	}
	
}
