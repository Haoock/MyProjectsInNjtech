package com.struts2.demo;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.List;

import javax.jws.Oneway;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.struts2.ServletActionContext;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.hibernate.demo.DBHelper;
import com.hibernate.demo.Info;
import com.hibernate.demo.Record;
import com.opensymphony.xwork2.ActionSupport;

public class GetJsonData extends ActionSupport {
	JSONArray jsay=new JSONArray();
	JSONArray jsuser=new JSONArray();
	public JSONArray getJsuser() {
		return jsuser;
	}
	public void setJsuser(JSONArray jsuser) {
		this.jsuser = jsuser;
	}
	public JSONArray getJsay() {
		return jsay;
	}
	public void setJsay(JSONArray jsay) {
		this.jsay = jsay;
	}
	DBHelper dbHelper;
	public String getTransactionDetails() throws IOException {
		dbHelper=new DBHelper();
		HttpServletRequest request=ServletActionContext.getRequest();
		int userid=(Integer)request.getSession().getAttribute("id");
		List<Record> list=dbHelper.getRecord(userid);
		System.out.println(list.size());
		
		for(int i=0;i<list.size();i++) {
			int Status=list.get(i).getStatus();
			String date=list.get(i).getDate()+" "+list.get(i).getTime();
			int money=list.get(i).getMoney();
			JSONObject jsonObject = new JSONObject();
			
			switch(Status) {
			case -2:
				jsonObject.put("id", "转出");
				break;
			case 2:
				jsonObject.put("id", "转入");
				break;
			case 1:
				jsonObject.put("id", "存入");
				break;
			case -1:
				jsonObject.put("id", "取出");
				break;
			}
	        
	        jsonObject.put("time", date);
	        jsonObject.put("money", money);
	        jsay.add(jsonObject);
		}
//		String result=JSON.toJSONString(list);
//		jsob=JSONObject.parseObject(result)(result);
		//传给jsp界面的值
		HttpServletResponse response=ServletActionContext.getResponse();
		response.setCharacterEncoding("utf-8");
		PrintWriter out = response.getWriter();
		out.println(jsay.toJSONString());
        out.flush(); 
        out.close();
		System.out.println(jsay.toJSONString());
		return "getTransSucc";
	}
	public String getUserInfoDetails() throws IOException{
		dbHelper=new DBHelper();
		HttpServletRequest request=ServletActionContext.getRequest();
		List<Info> list=dbHelper.getUserInfo();
		System.out.println(list.size());
		for(int i=0;i<list.size();i++) {
			int id=list.get(i).getAccountId();
			int balance=list.get(i).getBalance();
			String name=list.get(i).getName();
			String phone=list.get(i).getPhone();
			String address=list.get(i).getAddress();
			JSONObject jsonObject = new JSONObject();
			jsonObject.put("id", id);
			jsonObject.put("name", name);
			jsonObject.put("phone", phone);
			jsonObject.put("address", address);
			jsonObject.put("balance", balance);
			jsuser.add(jsonObject);
		}
		HttpServletResponse response=ServletActionContext.getResponse();
		response.setCharacterEncoding("utf-8");
		PrintWriter out = response.getWriter();
		out.println(jsuser.toJSONString());
        out.flush(); 
        out.close();
		System.out.println(jsuser.toJSONString());
		return "getUserinfoSucc";
	}
	
	
}
