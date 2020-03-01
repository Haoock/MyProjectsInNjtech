package com.web.haohao.dao;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

import com.web.haohao.entity.Admin;
import com.web.haohao.entity.Employee;

public class AdminDao {
	public boolean loginAdmin(Admin admin) {
		Connection conn = DbHelper.getConnection();
		String sql = "select * from user where name = ? and pwd = ?;";
		int res = 0;
		try {
			PreparedStatement pst = conn.prepareStatement(sql);
			pst.setString(1, admin.getUsername());
			pst.setString(2, admin.getPassword());
			System.out.println(pst.toString());
			ResultSet rs = pst.executeQuery();
			if (rs.next()) {
				res = 1;
			}
			pst.close();
			rs.close();
			if (res == 1) {
				return true;
			}
		} catch (SQLException e) {
			e.printStackTrace();
		}
		return false;
	}

	public List<Employee> getAllEmployees() {
		List<Employee> list = new ArrayList<Employee>();
		Connection conn = DbHelper.getConnection();
		String sql = "select * from employee;";
		try {
			PreparedStatement pst = conn.prepareStatement(sql);
			ResultSet rst = pst.executeQuery();
			while (rst.next()) {
				Employee employee = new Employee();
				employee.setName(rst.getString("name"));
				employee.setAge(rst.getInt("age"));
				employee.setSex(rst.getString("sex"));
				employee.setWorkyear(rst.getInt("workyear"));
				employee.setHiredate(rst.getString("hiredate"));
				employee.setLeavedate(rst.getString("leavedate"));
				employee.setJob(rst.getString("job"));
				list.add(employee);
			}
			rst.close();
			pst.close();
		} catch (SQLException e) {
			e.printStackTrace();
		}
		return list;
	}

	public boolean deleteEmpolyeeByName(String Name) {
		int res = 0;
		Connection conn = DbHelper.getConnection();
		String sql = "delete from employee where name=?";
		try {
			PreparedStatement pst = conn.prepareStatement(sql);
			pst.setString(1, Name);
			System.out.println(pst.toString());
			res = pst.executeUpdate();
			pst.close();
		} catch (SQLException e) {
			e.printStackTrace();
		}

		if (res == 0) {
			return false;
		} else {
			return true;
		}
	}

	public boolean addEmployee(Employee employee) {
		Connection connection = DbHelper.getConnection();
		String sql = "insert into employee values(?,?,?,?,?,?,?);";
		int res = 0;
		try {
			PreparedStatement pst = connection.prepareStatement(sql);
			pst.setString(1, employee.getName());
			pst.setInt(2, employee.getAge());
			pst.setString(3, employee.getSex());
			pst.setInt(4, employee.getWorkyear());
			pst.setString(5, employee.getHiredate());
			pst.setString(6, employee.getLeavedate());
			pst.setString(7, employee.getJob());
			res = pst.executeUpdate();
			pst.close();
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		if (res == 0) {
			return false;
		} else {
			return true;
		}
	}

	public Employee getEmployeeByName(String name) {
		Connection conn = DbHelper.getConnection();
		String sql = "select * from employee where name=?;";
		Employee employee = new Employee();
		try {
			PreparedStatement pst = conn.prepareStatement(sql);
			pst.setString(1, name);
			ResultSet rst = pst.executeQuery();

			while (rst.next()) {
				employee.setName(rst.getString("name"));
				employee.setAge(rst.getInt("age"));
				employee.setSex(rst.getString("sex"));
				employee.setWorkyear(rst.getInt("workyear"));
				employee.setHiredate(rst.getString("hiredate"));
				employee.setLeavedate(rst.getString("leavedate"));
				employee.setJob(rst.getString("job"));
			}
			rst.close();
			pst.close();
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return employee;
	}

	public boolean updateEmployee(Employee emp) {
		Connection conn = DbHelper.getConnection();
		String sql = "update employee set age=?,sex=?,workyear=?,hiredate=?,leavedate=?,job=? where name=?;";
		int res = 0;
		try {
			PreparedStatement pst = conn.prepareStatement(sql);
			pst.setInt(1, emp.getAge());
			pst.setString(2, emp.getSex());
			pst.setInt(3, emp.getWorkyear());
			pst.setString(4, emp.getHiredate());
			pst.setString(5, emp.getLeavedate());
			pst.setString(6, emp.getJob());
			pst.setString(7, emp.getName());
			res = pst.executeUpdate();
			pst.close();
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		if (res == 1) {
			return true;
		} else {
			return false;
		}
	}

}
