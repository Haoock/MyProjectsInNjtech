package com.struts2.interceptor;

import javax.servlet.http.HttpServletRequest;

import org.apache.struts2.ServletActionContext;

import com.opensymphony.xwork2.ActionInvocation;
import com.opensymphony.xwork2.interceptor.MethodFilterInterceptor;

/*
 * 配置拦截器：第一步继承MethodFilterInterceptor
 * 第二步：重写里面的doIntercept的方法
 * 第三步：配置interceptor和action的方法
 * 
 */
public class LoginInterceptor extends MethodFilterInterceptor{

	//此方法里面写拦截器逻辑
	@Override
	protected String doIntercept(ActionInvocation invocation) throws Exception {
		// TODO Auto-generated method stub
		HttpServletRequest request=ServletActionContext.getRequest();
		Object username=request.getSession().getAttribute("username");
		if(username!=null) {
			//登录状态，要做一个放行的操作
			System.out.println("放行");
			invocation.invoke();
		}else {
			//不是登录状态,不执行方法，返回登录界面
			//到配置页面中去找login的result中 
			System.out.println("失败不放行");
			return "login";
		}
		return null;
	}

}
