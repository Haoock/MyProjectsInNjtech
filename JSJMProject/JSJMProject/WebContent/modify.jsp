<%@ page language="java" contentType="text/html; charset=UTF-8"
	pageEncoding="UTF-8"%>
<%@ page import="com.web.haohao.entity.Employee"%>
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>修改员工信息</title>
<style type="text/css">
table {
	border: 0px solid black;
	margin: 0 auto;
}

td {
	width: 150px;
	border: 1px solid black;
	text-align: center;
}
</style>
</head>
<body style="text-align: center">
	<h2>修改员工信息</h2>
	<%Employee emp=(Employee)request.getAttribute("emp"); %>
	<form action="modifyServlet" name="form" method="post">
		<table>
			<tr>
				<td>员工姓名（只读）</td>
				<td><input type="text" name="name" value="<%=emp.getName()%>" onfocus="this.blur()"/></td>
			</tr>
			<tr>
				<td>员工年龄</td>
				<td><input type="text" name="age" value="<%=emp.getAge() %>" required /></td>
			</tr>
			<tr>
				<td>员工性别（只读）</td>
				<td ><input type="text" name="sex" value="<%=emp.getSex()%>" onfocus="this.blur()" /></td>
			</tr>
			<tr>
				<td>工龄</td>
				<td><input type="text" name="workyear" value="<%=emp.getWorkyear() %>" required /></td>
			</tr>
			<tr>
				<td>入职时间（只读）</td>
				<td><input type="text" name="hiredate" value="<%=emp.getHiredate()%>" onfocus="this.blur()"/></td>
			</tr>
			<tr>
				<td>离职时间</td>
				<td><input type="text" name="leavedate" value="<%=emp.getLeavedate() %>" required /></td>
			</tr>
			<tr>
				<td>员工职位</td>
				<td><input type="text" name="job" value="<%=emp.getJob() %>" required /></td>
			</tr>
			<tr>
				<td colspan="2"><button type="submit">保存</button></td>
			</tr>
		</table>
	</form>
</body>
</html>