package com.web.haohao.servlets;

import java.io.IOException;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import com.web.haohao.dao.AdminDao;
import com.web.haohao.entity.Employee;

/**
 * Servlet implementation class modifyServlet
 */
public class modifyServlet extends HttpServlet {
	private static final long serialVersionUID = 1L;
       
    /**
     * @see HttpServlet#HttpServlet()
     */
    public modifyServlet() {
        super();
        // TODO Auto-generated constructor stub
    }

	/**
	 * @see HttpServlet#doGet(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		// TODO Auto-generated method stub
		String method=request.getMethod();
		System.out.println("浏览器访问时采用的请求方式："+method);
		String name = request.getParameter("name");
		AdminDao dao=new AdminDao();
		Employee employee=dao.getEmployeeByName(name);
		request.setAttribute("emp", employee);
		request.getRequestDispatcher("modify.jsp").forward(request, response);
	}

	/**
	 * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse response)
	 */
	protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		// TODO Auto-generated method stub
		request.setCharacterEncoding("utf-8");
		String method=request.getMethod();
		System.out.println("浏览器访问时采用的请求方式："+method);
		String name=request.getParameter("name");
		String age=request.getParameter("age");
		String sex=request.getParameter("sex");
		String workyear=request.getParameter("workyear");
		String hiredate=request.getParameter("hiredate");
		String leavedate=request.getParameter("leavedate");
		String job=request.getParameter("job");
		Employee employee=new Employee();
		employee.setName(name);
		employee.setAge(Integer.parseInt(age));
		employee.setSex(sex);
		employee.setWorkyear(Integer.parseInt(workyear));
		employee.setHiredate(hiredate);
		employee.setLeavedate(leavedate);
		employee.setJob(job);
		AdminDao dao=new AdminDao();
		boolean res=dao.updateEmployee(employee);
		if(res) {
			request.getRequestDispatcher("showServlet").forward(request, response);
		}else {
			request.getRequestDispatcher("showServlet").forward(request, response);
		}
	}

}
