<%@ page language="java" contentType="text/html; charset=UTF-8"
	pageEncoding="UTF-8"%>
<%@ page import="java.util.*,com.web.haohao.entity.Employee"%>
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Insert title here</title>

    <style type="text/css">
            table {
                border: 1px solid black;
                margin: 0 auto;
            }
            
            td{
                width: 150px;
                border: 1px solid black;
                text-align: center;
            }
    </style>
</head>
<body style="text-align: center">
	<h2>员工管理系统</h2>
	<table border="1" cellspacing="0" rules="all">
	<tr>
		<th width="100px">姓名</th>
		<th width="100px">年龄</th>
		<th width="100px">性别</th>
		<th width="100px">工龄</th>
		<th width="100px">入职时间</th>
		<th width="100px">离职时间</th>
		<th width="100px">职位</th>
		<th width="100px">操作</th>
	</tr>
	<%
		ArrayList list = (ArrayList) request.getAttribute("list");
		for(int i=0;i<list.size();i++){
			Employee emp=(Employee)list.get(i);
	%>
	<tr valign="middle">
		<td><%=emp.getName() %></td>
		<td><%=emp.getAge() %></td>
		<td><%=emp.getSex() %></td>
		<td><%=emp.getWorkyear() %></td>
		<td><%=emp.getHiredate() %></td>
		<td><%=emp.getLeavedate() %></td>
		<td><%=emp.getJob() %></td>
		<td><a onclick="return checkconfirm()" href="deleteServlet?name=<%=emp.getName()%>">删除</a>|<a href="modifyServlet?name=<%=emp.getName()%>">修改</a></td>
	</tr>
	<%} %>
	<tr>
		<td colspan="8" style="text-align: left;"><a href="add.jsp">添加员工</a></td>
	</tr>
	</table>
</body>
<script>
	function checkconfirm(){
		var r = confirm("确定要删除吗？");
		if(r == true){
			return true;
		}else{
			return false;
		}
	}
</script>
</html>