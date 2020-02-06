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
    
    
    <div class="col-10" style="padding-left:25%; padding-top:5%;">
    	<div class="row">
			<form id="defaultForm" method="post" class="form-horizontal" action="${pageContext.request.contextPath}/userTransferMoney">
				<div class="form-group">
					<label class="col-lg- control-label">请输入转账目标的账号：</label>
		            <div class="col-lg-">
		            	<input type="text" class="form-control" name="tranferAccount" />
		            </div>
	            </div>
				<div class="form-group">
					<label class="col-lg- control-label">请输入转账金额：</label>
					<div class="col-lg-">
						<input type="text" class="form-control" name="transferMoney" />
					</div>
				</div>
				<div class="form-group">
					<div class="col-lg-9 col-lg-offset-3">
						<button type="submit" class="btn btn-primary" name="Submit" value="Submit">提交</button>
						<button type="button" class="btn btn-info" id="resetBtn">重置</button>
					</div>
				</div>
			</form>
			<!-- 
  			   <form id="defaultForm" method="post" class="form-horizontal" role="form" action="${pageContext.request.contextPath}/userTransferMoney">
                  <div class="form-group">
                    <label class="col-4 control-label">请输入转账目标的账号：</label>
                    <div class="col-10">
                      <input name="tranferAccount" type="text" class="form-control" id="tranferAccount" 
                           placeholder="目标账号">
                    </div>
                  </div>

                  <div class="form-group">
                    <label class="col-4 control-label">请输入转账金额：</label>
                    <div class="col-sm-10">
                      <input name="transferMoney" type="text" class="form-control" id="transferMoney" 
                           placeholder="转账金额">
                    </div>
                  </div>

                  <div class="form-group col-sm-offset-2 col-sm-10"">

                      <button type="submit" class="btn btn-primary">转账</button>

                  </div>
                </form>
                 -->
		</div>
	</div>
	</div>
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
					tranferAccount: 
					{
						validators: 
						{
							notEmpty: 
							{
								message: 'The target account is required and cannot be empty'
							},
							regexp: { //正则表达式
					            regexp: /^[0-9]+$/,
					            message: '目标账号只能包含数字'
		            		},
						}
					},
					transferMoney: 
					{
						validators: 
						{
							notEmpty: 
							{
								message: 'The transfer amount is required and cannot be empty'
							},
							regexp: { //正则表达式
					            regexp: /^[0-9_]+$/,
					            message: '转账金额只能包含数字'
		            		},
						}
					}
					
				}
				});
				$('#resetBtn').click(function() 
						{
							$('#defaultForm').data('bootstrapValidator').resetForm(true);
						});
			});
	 
    (function () {
  	    var flag=0;
  	    flag ='<%=request.getAttribute("flag")%>';
  	    console.log(flag);
  	    if(flag==1){
  	    	window.alert("转账成功！");
  	    }else{
  	      
  	    }
  	})();
    </script>
</body>
</html>