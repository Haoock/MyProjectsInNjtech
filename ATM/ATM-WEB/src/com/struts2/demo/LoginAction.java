package com.struts2.demo;

import javax.servlet.ServletContext;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import javax.websocket.Session;

import org.apache.struts2.ServletActionContext;

import com.hibernate.demo.DBHelper;
import com.hibernate.demo.Info;
import com.opensymphony.xwork2.ActionSupport;

public class LoginAction extends ActionSupport {
	DBHelper dbHelper;
	String depositMoney;
	String withdrawMoney;
	String transferMoney;
	String tranferAccount;
	String oldPassword;
	String newPassword;
public String getOldPassword() {
		return oldPassword;
	}
	public void setOldPassword(String oldPassword) {
		this.oldPassword = oldPassword;
	}
	public String getNewPassword() {
		return newPassword;
	}
	public void setNewPassword(String newPassword) {
		this.newPassword = newPassword;
	}
public String getTranferAccount() {
		return tranferAccount;
	}
	public void setTranferAccount(String tranferAccount) {
		this.tranferAccount = tranferAccount;
	}
public String getTransferMoney() {
		return transferMoney;
	}
	public void setTransferMoney(String transferMoney) {
		this.transferMoney = transferMoney;
	}
public String getWithdrawMoney() {
		return withdrawMoney;
	}
	public void setWithdrawMoney(String withdrawMoney) {
		this.withdrawMoney = withdrawMoney;
	}
public String getDepositMoney() {
		return depositMoney;
	}
	public void setDepositMoney(String depositMoney) {
		this.depositMoney = depositMoney;
	}
@Override
public String execute() throws Exception {
	
	
return SUCCESS;
	
}
public String userlogin() throws Exception{
		dbHelper=new DBHelper();
		// TODO Auto-generated method stub
		
		//使用ServletActionContext来获取表单数据
		//1:使用ServletActionContext来获取request对象
		HttpServletRequest request=ServletActionContext.getRequest();
		String username=request.getParameter("username");
		String password=request.getParameter("password");
		int status=dbHelper.GetLoginStatus(username, password);
		
		System.out.println(username+password);
		System.out.println(status);
		if(status!=-1) {
			//成功之后向session里面放入值
			int id=dbHelper.GetLoginId(username, password);
			request.getSession().setAttribute("username", username);
			request.getSession().setAttribute("id", id);
			
			if(status==0) {
				return "admin";
			}else {
				Info i = dbHelper.getUserInfo(id);
				String phone=i.getPhone();
				String address=i.getAddress();
				String accountName=i.getName();
				System.out.println(phone);
				System.out.println(username);
				System.out.println(address);
				request.setAttribute("userName", username);
				request.setAttribute("phone", phone);
				request.setAttribute("address", address);
				request.setAttribute("accountName", accountName);
				return "user";
			}
			//Servlet中总共可以设置三个域对象，
			//1:request域对象
			//			HttpServletRequest request=ServletActionContext.getRequest();
			//			request.setAttribute();
			//2:session域对象
			//			HttpSession session=request.getSession();
			//			session.setAttribute("name", "name");
			//3:ServletContext域对象
			//			ServletContext context=ServletActionContext.getServletContext();
			//			context.setAttribute(name, object);
		}
		else{
			request.setAttribute("flag", 1);
			return "fail";
		}
		
	}
public String userDeposit() throws Exception{
	dbHelper=new DBHelper();
	HttpServletRequest request=ServletActionContext.getRequest();
	int userid=(Integer)request.getSession().getAttribute("id");
	int money=Integer.parseInt(depositMoney);
	request.setAttribute("flag", 1);
	System.out.println("用户id");
	System.out.print(userid);
	System.out.print(money);
	dbHelper.deposit(userid, money, false);
	
	return "deposit";
}
public String userWithdraw() throws Exception{
	dbHelper=new DBHelper();
	HttpServletRequest request=ServletActionContext.getRequest();
	int userid=(Integer)request.getSession().getAttribute("id");
	int todayWithdraw = dbHelper.getTodayWithdraw(userid);
	
	int money=Integer.parseInt(withdrawMoney);
	System.out.println("用户id");
	System.out.print(userid);
	System.out.print(money);
	
	if(money + todayWithdraw > 20000 ) {
		System.out.println("取款失败，今日取款大于两万。");
		request.setAttribute("result", 2);
	} else {
		if( dbHelper.withdraw(userid, money, false)){
			System.out.println("取款成功！");
			request.setAttribute("result", 1);
		}
		else {
			System.out.println("取款失败，金额不足！");
			request.setAttribute("result", 0);
		}
		
		
	}

	return "withdraw";
}
public String userGetBalance() throws Exception{
	dbHelper=new DBHelper();
	HttpServletRequest request=ServletActionContext.getRequest();
	int userid=(Integer)request.getSession().getAttribute("id");
	Info i = dbHelper.getUserInfo(userid);
	int balance=i.getBalance();
	System.out.println(balance);
	request.setAttribute("balance", balance);
	return "getMoney";
	
}
public String userTransferMoney() throws Exception{
	dbHelper=new DBHelper();
	HttpServletRequest request=ServletActionContext.getRequest();
	int userid=(Integer)request.getSession().getAttribute("id");
	dbHelper.transfer(userid, Integer.parseInt(tranferAccount), Integer.parseInt(transferMoney));
	request.setAttribute("flag", 1);
	return "transferMoney";
}

public String userChangePass() throws Exception{
	dbHelper=new DBHelper();
	HttpServletRequest request=ServletActionContext.getRequest();
	int userid=(Integer)request.getSession().getAttribute("id");
	if(dbHelper.changePassword(userid, oldPassword, newPassword)) {
		request.setAttribute("result", 1);
		return "changeSucc";
	}else {
		request.setAttribute("result", 0);
		return "changeFail";
	}
}
public String userDeleteInfo() {
	dbHelper=new DBHelper();
	HttpServletRequest request=ServletActionContext.getRequest();
	String userid=request.getParameter("userId");
	System.out.println(userid);
	int id = Integer.parseInt(userid);
	dbHelper.deleteUser(id);
	return "deleteinfoSucc";
}
public String userWelcome() {
	dbHelper=new DBHelper();
	HttpServletRequest request=ServletActionContext.getRequest();
	int userid=(Integer)request.getSession().getAttribute("id");
	String username=dbHelper.getUsername(userid);
	Info i = dbHelper.getUserInfo(userid);
	request.setAttribute("userName", username);
	request.setAttribute("phone", i.getPhone());
	request.setAttribute("address", i.getAddress());
	request.setAttribute("accountName", i.getName());
	return "userwelcome";
	
}
 
}
