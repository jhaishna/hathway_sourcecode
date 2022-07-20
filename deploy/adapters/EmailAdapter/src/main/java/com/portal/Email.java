package com.portal;

public class Email {

	String ntf_id;
	String emailTo;
	String emailFrom;
	String subject;
	String message;
	byte[] blob;
	int status;
	int err_code;
	String err_desc;
	long modT;
			
	public long getModT() {
		return modT;
	}
	public void setModT(long modT) {
		this.modT = modT;
	}
	public int getStatus() {
		return status;
	}
	public void setStatus(int status) {
		this.status = status;
	}
	public int getErr_code() {
		return err_code;
	}
	public void setErr_code(int err_code) {
		this.err_code = err_code;
	}
	public String getErr_desc() {
		return err_desc;
	}
	public void setErr_desc(String err_desc) {
		this.err_desc = err_desc;
	}
	public byte[] getBlob() {
		return blob;
	}
	public void setBlob(byte[] blob) {
		this.blob = blob;
	}
	public String getNtf_id() {
		return ntf_id;
	}
	public void setNtf_id(String ntf_id) {
		this.ntf_id = ntf_id;
	}
	public String getEmailTo() {
		return emailTo;
	}
	public void setEmailTo(String emailTo) {
		this.emailTo = emailTo;
	}
	public String getEmailFrom() {
		return emailFrom;
	}
	public void setEmailFrom(String emailFrom) {
		this.emailFrom = emailFrom;
	}
	public String getSubject() {
		return subject;
	}
	public void setSubject(String subject) {
		this.subject = subject;
	}
	public String getMessage() {
		return message;
	}
	public void setMessage(String message) {
		this.message = message;
	}

}