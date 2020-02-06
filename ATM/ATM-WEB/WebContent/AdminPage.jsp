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
    	<a href="WelcomePage.jsp" class="navbar-brand col">佳生银行用户管理系统</a>
		<div class="container col justify-content-end">
            <div style="padding-right: 5%">
                <button type="button" class="btn btn-primary" data-toggle="modal" data-target="#myModal">
                 添加用户
                </button>
            </div>
   			<div class="btn-group">
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
    <!-- 模态框 -->
<div class="modal fade" id="myModal">
        <div class="modal-dialog">
          <div class="modal-content">
       
            <!-- 模态框头部 -->
            <div class="modal-header">
              <h4 class="modal-title">添加用户</h4>
              <button type="button" class="close" data-dismiss="modal">&times;</button>
            </div>
       
            <!-- 模态框主体 -->
            <div class="modal-body">
              <form action="${pageContext.request.contextPath}/adduser.action">
                <label for="demo">用户账号:</label>
                <div class="input-group mb-3">
                  <input type="text" class="form-control" placeholder="Accout" name="account">
                </div>
                <label for="demo">用户密码:</label>
                <div class="input-group mb-3">
                  <input type="password" class="form-control" placeholder="Password" name="password">
                </div>
                <label for="demo">用户初始金额:</label>
                <div class="input-group mb-3">
                  <input type="text" class="form-control" placeholder="InitialMoney" name="money">
                </div>
                <label for="demo">姓名:</label>
                <div class="input-group mb-3">
                  <input type="text" class="form-control" placeholder="Name" name="name">
                </div>
                <label for="demo">家庭住址:</label>
                <div class="input-group mb-3">
                  <input type="text" class="form-control" placeholder="Address" name="address">
                </div>
                <label for="demo">手机电话:</label>
                <div class="input-group mb-3">
                  <input type="text" class="form-control" placeholder="Phone" name="phone">
                </div>
                <button type="button" class="btn btn-secondary" data-dismiss="modal">关闭</button>
              <button class="btn btn-secondary" type="submit">完成</button>
              </form>
            </div>
            
            <!-- 模态框底部 
            <div class="modal-footer">
            </div>
            -->
       
          </div>
        </div>
      </div>
    <div>
        <table id="table2"  data-sortable="true">
        </table>
    </div>
    	<script src="https://cdn.bootcss.com/jquery/3.4.1/jquery.min.js"></script>
        <script src="https://cdn.staticfile.org/popper.js/1.15.0/umd/popper.min.js"></script>
        <script src="https://cdn.bootcss.com/twitter-bootstrap/4.3.1/js/bootstrap.min.js"></script>
        <script src="https://unpkg.com/bootstrap-table@1.15.3/dist/bootstrap-table.min.js"></script>
         <script>
         var $table=$('#table2').bootstrapTable({
            url: "<%=request.getContextPath()%>/getUserInfoDetails.action",
            method: 'get',
            showLoading: true,
            showRefresh: true,
            columns: [{
          field: 'id',
          title: 'AccountId',
        }, {
          field: 'name',
          title: '用户姓名'
        }, {
          field: 'phone',
          title: '电话'
        },  {
          field: 'address',
          title: '地址'
        }, {
          field: 'balance',
          title: '余额'
        },{
          field: 'action',
          title: '操作',
          width: 160,
          align: 'center',
          valign: 'middle',
          formatter: actionFormatter,
          events:{
        	  "click #deleterow":function(e,value,row,index){
        		  console.log(row.id);
        		  $.ajax({
        			  url:'${pageContext.request.contextPath}/userDeleteInfo.action',
        			  data:{
        			  userId : row.id,
        			  },
        			  type:'POST',
        			  error:function(){
        			  alert("更新用户信息失败");
        			  }
        			  });
        		  $table.bootstrapTable('remove', {
        		        field: 'id',
        		        values: [row.id]
        		      })
    		  }
          }
        }],
        sortName:"id",
        queryParams: "queryParams",                
        toolbar: "#toolbar",                
        sidePagination: "true",                
        striped: true, // 是否显示行间隔色                
        //search : "true",                
        uniqueId: "ID",                
        pageSize: "10",                
        pagination: true, // 是否分页                
        sortable: true,
          });
          
          function actionFormatter(value, row, index) {
              var id = value;
              var result = "";                             
              result += "<a href='javascript:;' id=\"deleterow\" class='btn btn-danger' οnclick=\"DeleteByIds('" + id + "')\" title='删除'>删除</a>";                
              return result;            
              }
          function deleteServerData(id){
        	  console.log(id);
          }
         
        		 
          
        
          </script>

</body>
</html>