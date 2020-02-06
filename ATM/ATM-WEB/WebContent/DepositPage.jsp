<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!DOCTYPE html>
<html>
<head>
  <title>佳生银行</title>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="bootstrap-4.3.1-dist/css/bootstrap.css">
  <link rel="stylesheet" href="https://cdn.bootcss.com/bootstrap-validator/0.5.3/css/bootstrapValidator.css"/>
  <script src="bootstrap-4.3.1-dist/others/jquery-3.4.1.js"></script>
  <script src="bootstrap-4.3.1-dist/others/popper.js"></script>
  <script src="bootstrap-4.3.1-dist/js/bootstrap.js"></script>
  <script type="text/javascript" src="https://cdn.bootcss.com/bootstrap-validator/0.5.3/js/bootstrapValidator.js"></script>  
</head>

<body>
    <nav class="navbar navbar-expand col">
    	<!-- Brand -->
    	<a href="userWelcome.action" class="navbar-brand col">佳生银行</a>
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

	    <div class="col-10" style="padding-left:10%; padding-top:5%;">
    	<div class="row">
    		<div class="col-10">
        	<form id="defaultForm" method="post" class="form-horizontal" action="${pageContext.request.contextPath}/userDeposit">
				<div class="form-group">
				<h1 style="color:dimgrey; padding-left: 5%">请输入或选择存款金额</h1>
            	<div class="row col" style="padding-top:5%">
            		<input type="text" class="form-control col-9" id="depositMoney" name="depositMoney" />
            		<button class="btn btn-primary col-3" type="submit" >Deal</button>
            	</div>
         		</div>
         	</form>
         	</div>
         	<div class="col-10">
                    	<div class="btn-toolbar" role="toolbar" style="padding-left:5% ;padding-top: 5%">
							<div class="btn-group" style="padding-left:10%" >
                    			<button type="button" onclick='document.getElementById("depositMoney").value="500"' class="btn btn-secondary">500</button>
                    		</div>
                    		<div class="btn-group" style="padding-left:10%">
								<button type="button" onclick='document.getElementById("depositMoney").value="1000"' class="btn btn-secondary">1000</button>
							</div>
							<div class="btn-group" style="padding-left:10%">
								<button type="button" onclick='document.getElementById("depositMoney").value="1500"' class="btn btn-secondary">1500</button>
                    		</div>
                    	</div>
                   </div>
                	</div><!-- /input-group -->      
            </div><!-- /.row -->
</div>
    <!--  
    <div class="col-10" style="padding-left:25%; padding-top:5%;">
            <div class="row">
                    <div class="col-lg-6">
                        <h1 style="color:dimgrey; padding-left: 5%">请输入存款金额</h1>
                        <form id="defaultForm" method="post" action="${pageContext.request.contextPath}/userDeposit">
	                        <div class="input-group" style="padding-top: 10%">
	                            <input type="text" name="depositMoney" class="form-control" placeholder="Deposit..."/>
	                            <span class="input-group-btn">
	                            <button class="btn btn-default" type="submit">Deal</button>
	                        </span>
	                    	</div>
                    	</form>
                </div><!-- /input-group -->      
           <!-- </div> /.row -->
    <!--  </div>-->
    <!--</div>-->
    -->
    <script type="text/javascript">
    $(document).ready(function() 
			{
			// Generate a simple captcha
			$('#defaultForm').bootstrapValidator({
			// live: 'disabled',
			message: 'This value is not valid',
			feedbackIcons: 
			{
				valid: 'glyphicon glyphicon-ok',
				invalid: 'glyphicon glyphicon-remove',
				validating: 'glyphicon glyphicon-refresh'
			},
			fields: 
			{
				depositMoney: 
				{
					validators: 
					{
						notEmpty: 
						{
							message: 'The password is required and cannot be empty'
						},
						regexp: { //正则表达式
				            regexp: /^[0-9_]+$/,
				            message: '存款金额只能包含数字'
	            		},
					}
				}
				
			}
			});
		});
    
    (function () {
  	    var flag=0;
  	    flag ='<%=request.getAttribute("flag")%>';
  	    console.log(flag);
  	    if(flag==1){
  	    	window.alert("存款成功！");
  	    }else{
  	      
  	    }
  	})();
    </script>
</body>
</html>
