<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!DOCTYPE html>
<html>
<head>
  <title>佳生银行</title>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://cdn.bootcss.com/twitter-bootstrap/4.3.1/css/bootstrap.min.css" rel="stylesheet">
  <script src="bootstrap-4.3.1-dist/others/jquery-3.4.1.js"></script>
  <script src="bootstrap-4.3.1-dist/others/popper.js"></script>
  <script src="bootstrap-4.3.1-dist/js/bootstrap.js"></script>
  <script src="test.css"></script>
</head>

<body>
    <nav class="navbar text-right col">
      <!-- Brand -->
      <a href="#" class="navbar-brand">佳生银行</a>
      ${sessionScope.username}
    </nav>

    <div class="col">
        <div style="background-color:slateblue;height:5vh;"> </div>
    </div>


    <div class="row">    
    <div class="col-2" style="padding-top: 2%">
      <!-- Menu -->
      <ul class="navbar-nav text-center">
        <li class="nav-item">
          <a class="nav-link" href="#DepositPage">存款</a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="#WithdrawPage">取款</a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="#TransferAccountPage">转账</a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="#CheckBalancePage">查询余额</a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="#TransactionDetailsPage">交易明细</a>
        </li>
        <li class="nav-item">
          <a class="nav-link" href="#ChangePasswordPage">修改密码</a>
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
                    <div class="col-lg-6">
                        <h1 style="color:dimgrey; padding-left: 5%">请输入存款金额</h1>
                        <div class="input-group" style="padding-top: 10%">
                            <input type="text" class="form-control" placeholder="Deposit...">
                            <span class="input-group-btn">
                            <button class="btn btn-default" type="button">Deal</button>
                        </span>
                    </div>
                </div><!-- /input-group -->      
            </div><!-- /.row -->
    
    </div>
</body>
</html>