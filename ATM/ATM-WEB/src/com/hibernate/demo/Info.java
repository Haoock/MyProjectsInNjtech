package com.hibernate.demo;

public class Info {
	private int AccountId;
	private int balance;
	private String name;
	private String phone;
	private String address;
	
	public int getAccountId() {
		return AccountId;
	}
	
	public void setAccountId(int accountid) {
		AccountId = accountid;
	}
	
	public int getBalance() {
		return balance;
	}
	
	public void setBalance(int balance) {
		this.balance = balance;
	}
	
	public String getName() {
		return name;
	}
	
	public void setName(String name) {
		this.name = name;
	}
	
	public String getPhone() {
		return phone;
	}
	
	public void setPhone(String phone) {
		this.phone = phone;
	}
	
	public String getAddress() {
		return address;
	}
	
	public void setAddress(String address) {
		this.address = address;
	}
}
