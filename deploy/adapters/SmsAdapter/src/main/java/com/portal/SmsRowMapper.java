package com.portal;

import java.io.IOException;
import java.io.StringReader;
import java.sql.ResultSet;
import java.sql.SQLException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.springframework.jdbc.core.RowMapper;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

public class SmsRowMapper implements RowMapper<Sms> {

	@Override
	public Sms mapRow(ResultSet rs, int rowNum) throws SQLException {

		Sms sms = new Sms();
		sms.setNtf_id(rs.getString("ntf_id"));
		sms.setBlob(rs.getBytes("SMS_FLIST_BUF_BUF"));

		String str = new String(rs.getBytes("SMS_FLIST_BUF_BUF"));	
    	Document doc;
    			
		try {
			doc = stringToDom(str);
	    	doc.getDocumentElement().normalize(); 
	    	sms.setMsisdn(doc.getElementsByTagName("MSISDN").item(0).getTextContent());
	    	sms.setMessage(doc.getElementsByTagName("MESSAGE").item(0).getTextContent());
		} catch (SAXException e) {
			e.printStackTrace();
		} catch (ParserConfigurationException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return sms;
	}
	
    public Document stringToDom(String xmlSource) 
            throws SAXException, ParserConfigurationException, IOException {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        DocumentBuilder builder = factory.newDocumentBuilder();
        return builder.parse(new InputSource(new StringReader(xmlSource)));
    }

}