package com.portal;

public class Sms {

	String ntf_id;
	String msisdn;
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
	public String getMsisdn() {
		return msisdn;
	}
	public void setMsisdn(String msisdn) {
		this.msisdn = msisdn;
	}
	public String getMessage() {
		return message;
	}
	public void setMessage(String message) {
		this.message = message;
	}

}