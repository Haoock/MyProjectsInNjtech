<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<title>欢迎界面</title>
</head>
<body>

	<h1>Hello World! Welcome to Struts.</h1>
	<div style="border:solid 1px black;width:300px;height:200px;margin:0 auto">
	<form action="hello" method="post">
	用户名 &nbsp;&nbsp;&nbsp;:<input type="text" name="name"/><br/> 
	年&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;龄:<input type="text" name="age"/><br/>
	Email&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;:<input type="text" name="email"/><br/>
	手机号码:<input type="text" name="phone"/><br/>
	<input type="submit" value="submit"/><br/>
	</form>
	</div>
</body>
</html>