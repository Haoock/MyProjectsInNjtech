package com.hibernate.demo;

public class User {
	private int AccountId;
	private String username;
	private String password;
	private int status;
	
	public int getAccountId() {
		return AccountId;
	}
	
	public void setAccountId(int id) {
		this.AccountId = id;
	}
	
	public String getUsername() {
		return username;
	}
	
	public void setUsername(String username) {
		this.username = username;
	}
	
	public String getPassword() {
		return password;
	}
	
	public void setPassword(String password) {
		this.password = password;
	}
	
	public int getStatus() {
		return this.status;
	}
	
	public void setStatus(int status) {
		this.status = status;
	}
	
}
