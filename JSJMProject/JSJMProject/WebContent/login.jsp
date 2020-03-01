<%@ page language="java" contentType="text/html; charset=UTF-8"
	pageEncoding="UTF-8"%>
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>注册页面</title>
<%
	if ("wrong".equals(request.getAttribute("errorMessage"))) {
%>
<script type="text/javascript">
	alert("用户名或者密码错误！");
</script>
<%
	} else {
%>
<%
	}
%>
</head>
<body style="text-align: center">
	<h3>Please login in</h3>
	<h3>${errorMessage}</h3>
	<form name="myform" action="loginServlet" method="POST">
		<label>账号</label> <input type="text" id="inputAccount" name="username"
			placeholder="用户名" required autofocus> <br /> <label>密码</label>
		<input type="password" id="inputPassword" name="password"
			placeholder="密码" required> <br />
		<button type="submit" onclick="return checkForm()">登录</button>
	</form>
</body>
<script type="text/javascript">
	function checkForm() {
		var form = document.forms["myform"];
		var username = form["username"].value;
		var password = form["password"].value;
		if (username == null || username == "") {
			alert("请输入用户名！");
			return false;
		} else if (password == null || password == "") {
			alert("请输入密码！");
			return false;
		}
		return true;
	}
</script>
</html>