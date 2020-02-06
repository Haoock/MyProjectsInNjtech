package com.struts2.demo;


import com.hibernate.demo.DBHelper;
import com.opensymphony.xwork2.ActionSupport;


public class UserAction extends ActionSupport {
	private String account;
	private String password;
	private String money;
	private String name;
	private String address;
	private String phone;
	DBHelper dbhelper=new DBHelper();

	//管理员添加用户
	@Override
	public String execute() throws Exception {//执行
		// TODO Auto-generated method stub
		System.out.println("account"+account);
		System.out.println("password"+password);
		System.out.println("money"+money);
		System.out.println("name"+name);
		System.out.println("address"+address);
		System.out.println("phone"+phone);
		int moneyNum=Integer.parseInt(money);
		dbhelper.RegistUser(account, password, 1, moneyNum, name, phone, address);
		return SUCCESS;
	}

	public String getAccount() {
		return account;
	}

	public void setAccount(String account) {
		this.account = account;
	}

	public String getPassword() {
		return password;
	}

	public void setPassword(String password) {
		this.password = password;
	}

	public String getMoney() {
		return money;
	}

	public void setMoney(String money) {
		this.money = money;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getAddress() {
		return address;
	}

	public void setAddress(String address) {
		this.address = address;
	}

	public String getPhone() {
		return phone;
	}

	public void setPhone(String phone) {
		this.phone = phone;
	}
	
}
