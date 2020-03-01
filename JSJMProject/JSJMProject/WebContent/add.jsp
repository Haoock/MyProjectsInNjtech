<%@ page language="java" contentType="text/html; charset=UTF-8"
	pageEncoding="UTF-8"%>
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>增加员工信息</title>
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
	<h2>添加员工</h2>
	<form action="addServlet" name="form">
		<table>
			<tr>
				<td>员工姓名</td>
				<td><input type="text" name="name" required/></td>
			</tr>
			<tr>
				<td>员工年龄</td>
				<td><input type="text" name="age" required/></td>
			</tr>
			<tr>
				<td>员工性别</td>
				<td><input type="text" name="sex" required/></td>
			</tr>
			<tr>
				<td>工龄</td>
				<td><input type="text" name="workyear" required/></td>
			</tr>
			<tr>
				<td>入职时间</td>
				<td><input type="text" name="hiredate" required/></td>
			</tr>
			<tr>
				<td>离职时间</td>
				<td><input type="text" name="leavedate" required/></td>
			</tr>
			<tr>
				<td>员工职位</td>
				<td><input type="text" name="job" required/></td>
			</tr>
			<tr>
				<td colspan="2"><button type="submit"
						onclick="return checkTable()">提交</button>
					<button type="reset">重置</button></td>
			</tr>
		</table>
	</form>
</body>
<script>
	function checkTable(){
		var form = document.forms["form"];
		var sex = form["sex"].value;
		if("男"==sex||"女"==sex){
			return true;
		}else{
			alert("性别请填写“男”或“女”");
			return false;
		}
		
	}
</script>
</html>