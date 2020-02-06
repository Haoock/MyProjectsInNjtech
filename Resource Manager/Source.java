package 资源管理器;

import java.io.*;
import javax.swing.*;//JTree的使用
import javax.swing.event.TreeExpansionEvent;
import javax.swing.event.TreeExpansionListener;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeNode;
import javax.swing.tree.TreePath;

import java.awt.*;
import java.net.*;//用来获取本机IP
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;


public class Source extends JFrame implements MouseListener{
	JTree mainTree;							//资源管理器中主树
	DefaultTreeModel treeModel;				//树的模型可以添加到主树中
	DefaultMutableTreeNode root;			//树的根节点
	DefaultMutableTreeNode selectionNode;	//当前选中的节点（树中的节点位置）
	
	Object [][] nodeList=new Object[0][0]; //用来存储节点的list
	
	InetAddress localAddress; 			//本地电脑的地址用来读取本地的资源
	
	JPopupMenu popUp=new JPopupMenu();	//当单机右键弹出来的子菜单
			
	String pathName;					//绝对名称和路径（磁盘的路径）
	String fileName;					//单独的文件名
	String copyFilename;				//复制文件的名称
	String copyPathname;				//复制文件所在的路径
	String goalPathname;				//目标的磁盘路径名
	
	
	
	
/**
 * @throws UnknownHostException
 * 构造函数
 */
Source() throws UnknownHostException {
		setPopup();
		this.setTitle("资源管理器");
		this.add(paneMake());
		this.setVisible(true);
		this.setPreferredSize(new Dimension(400,400));
		this.pack();
		setLocationRelativeTo(null);
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	}




/**
 * 右键弹出式菜单的构建
 */
public void setPopup() {
	
	JMenuItem mu1=new JMenuItem("复制");
	JMenuItem mu2=new JMenuItem("粘贴");
	JMenuItem mu3=new JMenuItem("删除");
	popUp.add(mu1);
	popUp.add(mu2);
	popUp.add(mu3);
	mu1.addActionListener(new popActionlistener());
	mu2.addActionListener(new popActionlistener());
	mu3.addActionListener(new popActionlistener());
}



/**
 * @author 浩
 *弹出式菜单功能的监听
 */
class popActionlistener implements ActionListener{

	@Override
	public void actionPerformed(ActionEvent e) {
		// TODO Auto-generated method stub
		if(e.getActionCommand()=="复制") {		//复制文件操作
			copyPathname=pathName;
			copyFilename=fileName;
			System.out.println("复制成功");
		}
		//粘贴文件操作
		if(e.getActionCommand()=="粘贴"&&(new File(pathName).isDirectory())) {
			goalPathname=pathName;
			goalPathname=goalPathname+"/"+copyFilename;
			System.out.println("新文件"+goalPathname);
			try {
				copyFile(copyPathname,goalPathname);
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}
			DefaultMutableTreeNode node2=new DefaultMutableTreeNode(copyFilename);
			selectionNode.add(node2);
			SwingUtilities.invokeLater(new Runnable(){ 
				public void run(){
					mainTree.updateUI();
			}});
		}
		if(e.getActionCommand()=="删除") {		//删除操作
			File file=new File(pathName);
			file.delete();
			treeModel.removeNodeFromParent(selectionNode);

			System.out.println("已成功删除");
		}
	}
	
}
/**
 * @param soursefile
 * @param goalfile
 * @throws IOException
 * 具体实现本地文件的复制和粘贴
 */
public void copyFile(String soursefile,String goalfile) throws IOException {
	File sourseFile=new File(soursefile);//源文件的获得
	File goalFile=new File(goalfile);	//目标文件的获得
		FileInputStream fis = new FileInputStream(sourseFile);
		FileOutputStream fos = new FileOutputStream(goalFile);
		
		BufferedInputStream bis=new BufferedInputStream(fis);
		BufferedOutputStream bos=new BufferedOutputStream(fos);

		int b;
	      while((b=bis.read())!=-1){
	       bos.write(b);
	    }
	      bis.close();
	      bos.close();
}
	




/**
 * @return
 * @throws UnknownHostException
 * 面板的构建
 */
public JPanel paneMake() throws UnknownHostException {
	
	JPanel mainPane=new JPanel();				//整体主要面板
	mainPane.setLayout(new BorderLayout());
	JScrollPane scrollPane = new JScrollPane(makeTreeStruct());
	scrollPane.setPreferredSize(new Dimension(200, 300));
	mainPane.add(scrollPane,BorderLayout.CENTER);
	return mainPane;
}



/**
 * @return
 * @throws UnknownHostException
 * 树结构的构造函数
 */
public JTree makeTreeStruct() throws UnknownHostException{
	localAddress=InetAddress.getLocalHost();
	root=new DefaultMutableTreeNode(localAddress.getHostName());
	for(char i='c';i<'g';i++) {
		String path=i+":/";
		File file=new File(path);
		DefaultMutableTreeNode discnode=new DefaultMutableTreeNode(path);
		root.add(discnode);
		readFiles(discnode,file);
		
	}
	treeModel=new DefaultTreeModel(root);
	mainTree=new JTree(treeModel);
	mainTree.setEditable(true);
	mainTree.setShowsRootHandles(false);
	mainTree.collapseRow(0);
	
	//树监听器的添加
	mainTree.addTreeSelectionListener(new TreeSelectionListener() {
	
		@Override
		public void valueChanged(TreeSelectionEvent e) {//对节点的监听
			// TODO Auto-generated method stub
			System.out.println("Node Valuechanged");
			
			selectionNode=(DefaultMutableTreeNode)(e.getPath().getLastPathComponent());
			TreeNode[] path=selectionNode.getPath();
			if(path==null)
				return;
			if(selectionNode.isRoot())
				return;
			
			pathName=path[1].toString();
			for(int i=2;i<path.length;i++)
				pathName=pathName+"/"+path[i].toString();
			fileName=path[path.length-1].toString();
			File file=new File(pathName);
			System.out.println("所选文件夹或文件夹名："+fileName);
			SwingUtilities.invokeLater(new Runnable(){ 
				public void run(){
					mainTree.updateUI();
			}});
			selectionNode.removeAllChildren();
			readFiles(selectionNode,file);
			
			
		}
	});
	
	//展开树节点监听器的添加
	mainTree.addTreeExpansionListener(new TreeExpansionListener() {

		@Override
		public void treeCollapsed(TreeExpansionEvent e) {
			// TODO Auto-generated method stub
			
		}

		@Override
		
		public void treeExpanded(TreeExpansionEvent e) {
			// TODO Auto-generated method stub
			
			System.out.println("expend");
		}
		
	});
	
	mainTree.addMouseListener(this);
	
	return mainTree;
}




/**
 * @param childNode
 * @param parent
 * 读取本地文件
 */
public void readFiles(DefaultMutableTreeNode childNode,File parent)
{
	if(parent.isDirectory()) {				//如果父节点是文件夹的话
	File [] list=parent.listFiles();
	for(int j=0;j<list.length;j++) {
		if(list[j].isDirectory()) {
			String nodename=list[j].getName();
			File nu=null;
			DefaultMutableTreeNode sbparent=new DefaultMutableTreeNode(nodename);
			DefaultMutableTreeNode nul=new DefaultMutableTreeNode(nu);
			sbparent.add(nul);
			childNode.add(sbparent);
		}
		else {
			String str=list[j].getName();
			DefaultMutableTreeNode child=new DefaultMutableTreeNode(str);
			childNode.add(child);
		}
	}
	}
}
//main函数
public static void main(String[] args) throws Exception {
	new Source();
}

//鼠标的自动生成监听函数这里只用到了点击函数
@Override
public void mouseClicked(MouseEvent e) {
	// TODO Auto-generated method stub
	TreePath path=mainTree.getPathForLocation(e.getX(), e.getY());
	//DefaultMutableTreeNode node=(DefaultMutableTreeNode)e.getSource();
	if(e.getButton()==MouseEvent.BUTTON3&&path!=null)
	{
		popUp.show(e.getComponent(), e.getX(), e.getY());
	}
}
@Override
public void mouseEntered(MouseEvent arg0) {
	// TODO Auto-generated method stub
}
@Override
public void mouseExited(MouseEvent arg0) {
	// TODO Auto-generated method stub
}
@Override
public void mousePressed(MouseEvent arg0) {
	// TODO Auto-generated method stub
}
@Override
public void mouseReleased(MouseEvent arg0) {
	// TODO Auto-generated method stub
}
}

