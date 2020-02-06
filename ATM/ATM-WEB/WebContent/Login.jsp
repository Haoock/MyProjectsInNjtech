<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
    <meta name="description" content="">
    <title>loginPage</title>
    <link rel="canonical" href="https://v4ing.bootcss.com/docs/4.3/examples/sign-in/">
    <!-- Bootstrap core CSS -->
    <link href="https://cdn.bootcss.com/twitter-bootstrap/4.3.1/css/bootstrap.min.css" rel="stylesheet">
    <link href="https://cdn.bootcss.com/font-awesome/5.10.2/css/fontawesome.min.css" rel="stylesheet">
    <style>
      .bd-placeholder-img {
        font-size: 1.125rem;
        text-anchor: middle;
        -webkit-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
      }

      @media (min-width: 768px) {
        .bd-placeholder-img-lg {
          font-size: 3.5rem;
        }
      }
    </style>

    <!-- Custom styles for this template -->
    <link href="${pageContext.request.contextPath}/css/signin.css" rel="stylesheet">
    <link href="https://cdn.bootcss.com/font-awesome/5.10.2/css/solid.min.css" rel="stylesheet">
  	
  </head>
  <body class="text-center" onload="init()">
    <form class="form-signin" action="${pageContext.request.contextPath}/userlogin.action">
  <i class="fas fa-money-check" ></i>
  <h1 class="h3 mb-3 font-weight-normal">Please login in</h1>
   <div id="wrong" class="alert alert-danger">
    <strong>账号或密码错误！</strong> 
  </div>
  <label for="inputAccount" class="sr-only">Account</label>
  <input type="text" id="inputAccount" class="form-control" name="username" placeholder="Account" required autofocus>
  <label for="inputPassword" class="sr-only">Password</label>
  <input type="password" id="inputPassword" class="form-control" name="password" placeholder="Password" required>
  <div class="checkbox mb-3">
    <label>
      <input id="rua" type="checkbox" value="remember-me" > Remember me
    </label>
  </div>
  <button class="btn btn-lg btn-primary btn-block" type="submit" onclick="RememberMeIn()">Login in</button>
</form>
<script src="https://cdn.bootcss.com/jquery/3.4.1/jquery.min.js"></script>
<script src="https://cdn.bootcss.com/popper.js/1.15.0/esm/popper.min.js"></script>
<script>
function init()
{
	RememberMeOut();
}

function setCookie(CookieName,CookieValue,exdays)
{
  var d = new Date();
  d.setTime(d.getTime()+(exdays*24*60*60*1000));
  var expires = "expires="+d.toGMTString();
  document.cookie = CookieName + "=" + CookieValue + "; " + expires;
}


function getCookie(CookieName)
{
  var name = CookieName + "=";
  var ca = document.cookie.split(';');
  for(var i=0; i<ca.length; i++) 
  {
    var c = ca[i].trim();
    if (c.indexOf(name)==0) return c.substring(name.length,c.length);
  }
  return "";
}

function RememberMeIn()
{
	var rua = document.getElementById('rua');
	if( rua.checked )
	{
		var username = document.getElementById("inputAccount").value;
		setCookie("username",username,7);
		var password = document.getElementById("inputPassword").value;
	    setCookie("password",password,7);
	}
	else
	{
		setCookie("username",username,-1);
	    setCookie("password",password,-1);
	}
    
}

function RememberMeOut()
{
	var username=getCookie("username");
	var password=getCookie("password");
	document.getElementById("inputAccount").value = username;
	document.getElementById("inputPassword").value = password;
}

(function () {
	  console.log('do something');
	  $("#wrong").hide();
	    var flag=0;
	    flag = '<%=request.getAttribute("flag")%>';
	    console.log(flag);
	    if(flag==1){
	      $("#wrong").show();
	    }else{
	      $("#wrong").hide();
	    }
	})();
</script>

</body>
</html>