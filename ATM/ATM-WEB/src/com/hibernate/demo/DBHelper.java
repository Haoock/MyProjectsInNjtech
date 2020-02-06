package com.hibernate.demo;

import java.util.List;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.cfg.Configuration;
import org.hibernate.query.Query;

public class DBHelper {
	
	//用户注册
	public void RegistUser(String username, String password, int status, int balance, String name, String phone, String address) {
        // 使用Hibernate的API来完成将Customer信息保存到mysql数据库中的操作
        Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        //创建一个新user对象
        User u = new User();
        Info info = new Info();
        u.setUsername(username);
        u.setPassword(password);
        u.setStatus(status);
        info.setAddress(address);
        info.setBalance(balance);
        info.setName(name);
        info.setPhone(phone);
        //操作
        session.save(u);
        session.save(info);
        
        //事务提交
        session.getTransaction().commit();
        session.close();
        sessionFactory.close();   
	}
	
	//用户删除
	public void deleteUser(int id) {
		Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        User u = session.get(User.class, id);
        Info i = session.get(Info.class, id);
        session.delete(u);
        session.delete(i);
        
        //事务提交
        session.getTransaction().commit();
        session.close();
        sessionFactory.close();
	}
	
	//修改用户信息
	public void updapteUserName(int id, String newName) {
		Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        Info i = session.get(Info.class, id);
        i.setName(newName);
        session.update(i);
        
        //事务提交
        session.getTransaction().commit();
        session.close();
        sessionFactory.close();
	}
	
	public void updapteUserPhone(int id, String newPhone) {
		Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        Info i = session.get(Info.class, id);
        i.setPhone(newPhone);
        session.update(i);
        
        //事务提交
        session.getTransaction().commit();
        session.close();
        sessionFactory.close();
	}
	
	public void updapteUserAddress(int id, String newAddress) {
		Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        Info i = session.get(Info.class, id);
        i.setAddress(newAddress);
        session.update(i);
        
        //事务提交
        session.getTransaction().commit();
        session.close();
        sessionFactory.close();
	}
	
	//存取款
	//存款
	public void deposit(int id, int money, boolean isTrans) {
		Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        Info i = session.get(Info.class, id);
        i.setBalance( i.getBalance() + money );
        session.update(i);
        
        if(!isTrans) {
        	Record r = new Record();
            r.setAccountId(id);
            r.setMoney(money);
		    SimpleDateFormat TimeFormatter = new SimpleDateFormat("HH:mm:ss");
			SimpleDateFormat DateFormatter = new SimpleDateFormat("yyyy/MM/dd");
		    Date date = new Date(System.currentTimeMillis());
			r.setTime(TimeFormatter.format(date));
			r.setDate(DateFormatter.format(date));
			r.setStatus(1);
			System.out.println(r.getDate()+r.getTime());
			session.save(r);
        }
        
        //事务提交
        session.getTransaction().commit();
        session.close();
        sessionFactory.close();
	}
	
	public boolean withdraw(int id, int money, boolean isTrans) {
		Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        Info i = session.get(Info.class, id);
        if(i.getBalance() < money) {
        	//事务完结
            session.close();
            sessionFactory.close();
            
            return false;
        }
        else{
        	i.setBalance( i.getBalance() - money );
        	session.update(i);
        
        	if(!isTrans) {
            	Record r = new Record();
                r.setAccountId(id);
                r.setMoney(money);
    		    SimpleDateFormat TimeFormatter = new SimpleDateFormat("HH:mm:ss");
    			SimpleDateFormat DateFormatter = new SimpleDateFormat("yyyy/MM/dd");
    		    Date date = new Date(System.currentTimeMillis());
    			r.setTime(TimeFormatter.format(date));
    			r.setDate(DateFormatter.format(date));
    			r.setStatus(-1);
    			session.save(r);
            }
        	
        	//事务提交
        	session.getTransaction().commit();
        	session.close();
        	sessionFactory.close();
        	
        	return true;
        }
	}
	
	//转账
	public boolean transfer(int userId, int targetId, int money) {
		Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        boolean isMoneyEnough = withdraw(userId, money, true);
        if(!isMoneyEnough) {
        	return false;
        } else {
        	deposit(targetId, money, true);
        	
        	Record userRecord = new Record();
        	userRecord.setAccountId(userId);
        	userRecord.setMoney(money);
		    SimpleDateFormat TimeFormatter = new SimpleDateFormat("HH:mm:ss");
			SimpleDateFormat DateFormatter = new SimpleDateFormat("yyyy/MM/dd");
		    Date date = new Date(System.currentTimeMillis());
		    userRecord.setTime(TimeFormatter.format(date));
		    userRecord.setDate(DateFormatter.format(date));
		    userRecord.setStatus(-2);
			session.save(userRecord);
			
			Record targetRecord = new Record();
			targetRecord.setAccountId(targetId);
			targetRecord.setMoney(money);
			targetRecord.setTime(TimeFormatter.format(date));
			targetRecord.setDate(DateFormatter.format(date));
			targetRecord.setStatus(2);
			session.save(targetRecord);
			
			//事务提交
        	session.getTransaction().commit();
        	session.close();
        	sessionFactory.close();
			
			return true;
        }
	}
	
	//登陆
	public int GetLoginStatus(String username, String password) {
		// 使用Hibernate的API来完成将Customer信息保存到mysql数据库中的操作
        Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        Query q = session.createQuery("from User where username=\'" + username + "\' and password=\'" + password +"\'");
        List<User> list= q.list();
        
        if(list.isEmpty()) {
        	session.close();
            sessionFactory.close();
        	return -1;
        }
        else {
        	User u = list.get(0);
        	System.out.println(u.getStatus());
        	session.close();
            sessionFactory.close();
        	return u.getStatus();
        }
	}
	
	//获取登录用户ID
	public int GetLoginId(String username, String password) {
		// 使用Hibernate的API来完成将Customer信息保存到mysql数据库中的操作
        Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        Query q = session.createQuery("from User where username=\'" + username + "\' and password=\'" + password +"\'");
        List<User> list= q.list();
        
        User u = list.get(0);
        
        session.close();
        sessionFactory.close();
        
        return u.getAccountId();
	}
	
	//查询用户信息
	public Info getUserInfo(int id) {
		Info i = new Info();
		// 使用Hibernate的API来完成将Customer信息保存到mysql数据库中的操作
        Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        i = session.get(Info.class, id);
        
        // 事务提交
        session.getTransaction().commit();
        session.close();
        sessionFactory.close();
        
		return i;
	}
	
	//修改密码
	public boolean changePassword(int id, String oldPassword, String newPassword) {
		// 使用Hibernate的API来完成将Customer信息保存到mysql数据库中的操作
        Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        User u = session.get(User.class, id);
        if(oldPassword.equals(u.getPassword())) {
        	u.setPassword(newPassword);
            session.update(u);
            
            //事务提交
            session.getTransaction().commit();
            session.close();
            sessionFactory.close();
            
            return true;
        } else {
        	//事务提交
        	System.out.println("旧密码错误");
            session.close();
            sessionFactory.close();
            
        	return false;
        }
        
        
      
	}
	
	//查询记录
	public List<Record> getRecord(int id) {
		// 使用Hibernate的API来完成将Customer信息保存到mysql数据库中的操作
        Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
		Query q = session.createQuery("from Record where AccountId=" + id);
        List<Record> list= q.list();
        
        session.close();
        sessionFactory.close();
        
        return list;
	}
	
	public int getTodayWithdraw(int userid) {
		// 使用Hibernate的API来完成将Customer信息保存到mysql数据库中的操作
        Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        SimpleDateFormat DateFormatter = new SimpleDateFormat("yyyy/MM/dd");
	    Date date = new Date(System.currentTimeMillis());
	    String todayDate = DateFormatter.format(date);
		
        Query q = session.createQuery("from Record where AccountId=" + userid + " and Date=\'" + todayDate + "\' and status = -1");
        
        int result = 0;
        
        List<Record> list = q.list();
        for(int i=0; i<list.size(); i++) {
        	result += list.get(i).getMoney();
        }
        
        System.out.println("今日取款金额：" + result);
        
        session.close();
        sessionFactory.close();
        
        return result;
	}
	
	public List<Info> getUserInfo(){
		// 使用Hibernate的API来完成将Customer信息保存到mysql数据库中的操作
        Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        Query q = session.createQuery("from Info where AccountId != 1");
        List<Info> list = q.list();
        
        session.close();
        sessionFactory.close();
        return list;
	}
	
	public String getUsername(int id) {
		// 使用Hibernate的API来完成将Customer信息保存到mysql数据库中的操作
        Configuration config = new Configuration().configure(); // Hibernate框架加载hibernate.cfg.xml文件
        SessionFactory sessionFactory = config.buildSessionFactory();
        Session session = sessionFactory.openSession(); // 相当于得到一个Connection
        // 开启事务
        session.beginTransaction();
        
        User user = session.get(User.class, id);
        
        session.close();
        sessionFactory.close();
        
        return user.getUsername();
	}
}