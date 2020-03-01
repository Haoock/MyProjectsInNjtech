package com.web.haohao.servlets;

import java.io.IOException;
import java.util.Enumeration;

import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import com.web.haohao.dao.AdminDao;
import com.web.haohao.entity.Admin;

/**
 * Servlet implementation class loginServlet
 */
public class loginServlet extends HttpServlet {
	private static final long serialVersionUID = 1L;
       
    /**
     * @see HttpServlet#HttpServlet()
     */
    public loginServlet() {
        super();
        // TODO Auto-generated constructor stub
    }

	/**
	 * @see HttpServlet#doGet(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		// TODO Auto-generated method stub
//		response.getWriter().append("Served at: ").append(request.getContextPath());
		request.setCharacterEncoding("utf-8");
		StringBuffer url= request.getRequestURL();
		System.out.println("浏览器访问的资源地址"+url);
		String method=request.getMethod();
		System.out.println("浏览器访问时采用的请求方式："+method);
		
		Enumeration paramNames = request.getParameterNames();
		while(paramNames.hasMoreElements()) {
			String paramName=(String)paramNames.nextElement();
			System.out.println("携带的请求参数："+paramName);
		}
		
		
		String username=request.getParameter("username");
		String password=request.getParameter("password");
		Admin admin =new Admin();
		admin.setUsername(username);
		admin.setPassword(password);
		AdminDao dao=new AdminDao();
		boolean res=dao.loginAdmin(admin);
		if(res) {
			request.getRequestDispatcher("showServlet").forward(request, response);
		}else {
			request.setAttribute("errorMessage", "wrong");
			request.getRequestDispatcher("/login.jsp").forward(request, response);
		}
		
	}


	/**
	 * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		// TODO Auto-generated method stub
		doGet(request, response);
	}

}
