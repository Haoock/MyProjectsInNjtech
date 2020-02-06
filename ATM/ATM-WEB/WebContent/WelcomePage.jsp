<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!DOCTYPE html>
<html>
<head>
  <title>佳生银行</title>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="bootstrap-4.3.1-dist/css/bootstrap.css">
</head>

<body>
    <nav class="navbar navbar-expand col">
    	<!-- Brand -->
    	<a href="WelcomePage.jsp" class="navbar-brand col">佳生银行</a>
		<div class="container col justify-content-end">
   			<div class="dropdown">
    			<button type="button" class="btn btn-primary dropdown-toggle" data-toggle="dropdown">${sessionScope.username}</button>
    			<div class="dropdown-menu">
      			<a class="dropdown-item" href="Login.jsp">退出</a>
    			</div>
  			</div>
	</div>
    </nav>
    

    <div class="col">
        <div style="background-color:slateblue;height:5vh;"> </div>
    </div>


    <div class="row">    
    <div class="col-2" style="padding-top: 2%">
      <!-- Menu -->
      <ul class="navbar-nav text-center">
        <li class="nav-item">
          <a class="nav-link" href="DepositPage.jsp">存款</a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="WithdrawPage.jsp">取款</a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="TransferAccountPage.jsp">转账</a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="userGetBalance.action">查询余额</a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="TransactionDetailsPage.jsp">交易明细</a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="ChangePasswordPage.jsp">修改密码</a>
        </li>
      </ul>

      <div class="DepositPage" id="DepositPage"></div>
      <div class="WithdrawPage" id="WithdrawPage"></div>
      <div class="TransferAccountPage" id="TransferAccountPage"></div>
      <div class="CheckBalancePage" id="CheckBalancePage"></div>
      <div class="TransactionDetailsPage" id="TransactionDetailsPage"></div>
      <div class="ChangePasswordPage" id="ChangePasswordPage"></div>
    </div>

    <div class="col-10" style="padding-top:5%;padding-left:1%;padding-right:10%;">
       <div class="row">
         <div class="col-lg-6">
           <table class="table text-center">
	           	<thead>
	            	<tr>
	                	<th>账号</th>
	                </tr>
	            </thead>
	            <tbody>
	            	<tr>
	                  	<td>${userName}</td>
	                </tr>
	            </tbody>
           </table>
         <table class="table text-center">
	           	<thead>
	            	<tr>
	                	<th>手机</th>
	                </tr>
	            </thead>
	            <tbody>
	            	<tr>
	                  	<td>${phone}</td>
	                </tr>
	            </tbody>
           </table>
           
           </div>
           <div class="col-lg-6">
             <table class="table text-center">
	           	<thead>
	            	<tr>
	                	<th>姓名</th>
	                </tr>
	            </thead>
	            <tbody>
	            	<tr>
	                  	<td>${accountName}</td>
	                </tr>
	            </tbody>
           </table>
           <table class="table text-center">
	           	<thead>
	            	<tr>
	                	<th>住址</th>
	                </tr>
	            </thead>
	            <tbody>
	            	<tr>
	                  	<td>${address}</td>
	                </tr>
	            </tbody>
           </table>
          </div>
        </div>
      </div>
    </div>

    </div>
    <script src="https://cdn.bootcss.com/jquery/3.4.1/jquery.min.js"></script>
    <script src="https://cdn.bootcss.com/popper.js/1.15.0/esm/popper.min.js"></script>
    <script src="https://cdn.bootcss.com/twitter-bootstrap/4.3.1/js/bootstrap.min.js"></script>

</body>
</html>
