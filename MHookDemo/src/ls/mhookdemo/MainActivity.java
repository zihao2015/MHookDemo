package ls.mhookdemo;
import java.io.*;
import java.util.*;

import ls.client.TimeLog;
import ls.hook.Root;
import ls.mhookdemo.R;
import android.view.*;
import android.view.View.OnClickListener;
import android.widget.*;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.AdapterView.OnItemClickListener;
import android.annotation.SuppressLint;

public class MainActivity extends Activity {
	private List<PackageInfo> listdatas = new ArrayList<PackageInfo>();
	private ListView mlistview ;
	private Button button_Substrace;
	private Button button_reset;
	PackageManager pManager; 
	public static String FILE_PATH;
	public static String filter_app;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);	
		TimeLog.Log();
		
		Root.upgradeRootPermission(getPackageCodePath());
		FILE_PATH = "/sdcard/MHookDemo/";
		File mfile = new File(FILE_PATH);
		if(!mfile.exists()){
			mfile.mkdir();
		}
		FILE_PATH += "Config.txt";
		pManager = getPackageManager();
		mlistview = (ListView)findViewById(R.id.list_app);
		button_Substrace = (Button)findViewById(R.id.button_Substrace);
		button_reset =  (Button)findViewById(R.id.button_Reset);
		listdatas = getAllApps(MainActivity.this);
		final MyListAdapter myListAdapter = new MyListAdapter(listdatas);
		mlistview.setAdapter(myListAdapter);
		mlistview.setOnItemClickListener(new OnItemClickListener() {
			@SuppressLint("ShowToast")
			@Override
			public void onItemClick(AdapterView<?> parent, View view,
					int position, long id) {
				ViewHolder viewHolder = (ViewHolder)view.getTag();
				viewHolder.selected.toggle();
				myListAdapter.map.put(position, viewHolder.selected.isChecked());				
			}
		});
		
		button_Substrace.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				StringBuilder mBuilder = new StringBuilder();
				for (int i = 0; i < listdatas.size(); i++) {
					if (myListAdapter.map.get(i)) {
						mBuilder.append(listdatas.get(i).packageName+",");
					}
				}
				System.out.println(mBuilder.toString());
				try {
					writeString(mBuilder.toString(), FILE_PATH);
					Toast.makeText(MainActivity.this, "写入成功", Toast.LENGTH_SHORT).show();
					//利用CP直接复制
					Root.SuCpoyShell("/data/local/",FILE_PATH);
					Root.upgradeRootPermission("/data/local/Config.txt");
				} catch (IOException e) {
					System.out.println("写入文件失败");
					Toast.makeText(MainActivity.this, "文件写入失败", Toast.LENGTH_SHORT).show();
					e.printStackTrace();
				}
			}
		});
		TimeLog.Log();
		button_reset.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
			}
		});
	}
/**
 * 
 * @param context
 * @return
 */
	 // 获取手机内所有应用
	public static List<PackageInfo> getAllApps(Context context) {
		 
        List<PackageInfo> apps = new ArrayList<PackageInfo>();
        PackageManager pManager = context.getPackageManager();
       
        List<PackageInfo> packlist = pManager.getInstalledPackages(0);
        for (int i = 0; i < packlist.size(); i++) {
            PackageInfo pak = (PackageInfo) packlist.get(i);
            //apps.add(pak);
            // 判断是否为非系统预装的应用程序
            // 这里还可以添加系统自带的，这里就先不添加了，如果有需要可以自己添加
            // if()里的值如果<=0则为自己装的程序，否则为系统工程自带
            if ((pak.applicationInfo.flags & pak.applicationInfo.FLAG_SYSTEM) <= 0) {
                // 添加自己已经安装的应用程序
                apps.add(pak);
            }/**/
            
        }
        return apps;
    }
	//自定义ListView适配器  
    class MyListAdapter extends BaseAdapter{  
      List<PackageInfo> listPerson;  
      public  HashMap<Integer,Boolean> map;   
          
        @SuppressLint("UseSparseArrays")
		public MyListAdapter(List<PackageInfo> list){  
            listPerson = new ArrayList<PackageInfo>();  
            listPerson = list;  
            map = new HashMap<Integer,Boolean>();
            filter_app = readFileByLines(FILE_PATH);
            for (int i = 0; i < list.size(); i++) {
            	if (filter_app.contains(list.get(i).packageName)) {
				map.put(i, true);
            	}
            	else {
            		map.put(i, false);
					
				}
			}
        }  
  
        @Override  
        public int getCount() {  
            return listPerson.size();  
        }  
  
        @Override  
        public Object getItem(int position) {  
            return listPerson.get(position);  
        }  
  /*
   * (non-Javadoc)
   * @see android.widget.Adapter#getItemId(int)
   */
        @Override  
        public long getItemId(int position) {  
            return position;  
        }  
  /*
   * (non-Javadoc)
   * @see android.widget.Adapter#getView(int, android.view.View, android.view.ViewGroup)
   */
        @Override  
        public View getView(int position, View convertView, ViewGroup parent) {  
            ViewHolder holder = null;  
              
            if (convertView == null) {  
                  
                LayoutInflater mInflater = (LayoutInflater)   
                        getSystemService(Context.LAYOUT_INFLATER_SERVICE);  
                convertView = mInflater.inflate(R.layout.listitem, null);  
                holder = new ViewHolder();  
                holder.selected = (CheckBox)convertView.findViewById(R.id.list_select);  
                holder.name = (TextView)convertView.findViewById(R.id.list_name);  
                convertView.setTag(holder);  
            }else{  
                holder = (ViewHolder)convertView.getTag();  
            }  
              
            holder.selected.setChecked(map.get(position));  
            holder.name.setText(pManager.getApplicationLabel(listPerson.get(position).applicationInfo));  
              
            return convertView;  
        }  
          
    }  
      
    static class ViewHolder{  
        CheckBox selected;  
        TextView name;  
    }  
    /**
     * 
     * @param fileName
     * @return
     */
	public  static String readFileByLines(String fileName) {
		File file = new File(fileName);
		if (!file.exists()) {
			try {
				file.createNewFile();
				file.setReadable(true, false);
				file.setWritable(true, false);
				
			} catch (IOException e) {

				e.printStackTrace();
			}
		}
		BufferedReader reader = null;
		StringBuilder builder = new StringBuilder();
		try {
			reader = new BufferedReader(new FileReader(file));
			String tempString = null;
			while ((tempString = reader.readLine()) != null) {
				builder.append(tempString);// +System.getProperties().getProperty("line.separator")
			}
			reader.close();
			return builder.toString();
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			if (reader != null) {
				try {
					reader.close();
				} catch (IOException e1) {
				}
			}
		}
		return null;
	}
/**
 * 
 * @param string
 * @param filename
 * @throws IOException
 */
	public void writeString(String string, String filename) throws IOException {
		try {
			
			FileWriter writer = new FileWriter(filename, false);
			writer.write(string);
			writer.close();
		} catch (IOException e) {
			Toast.makeText(MainActivity.this, "文件创建失败", Toast.LENGTH_SHORT).show();
			e.printStackTrace();
		}
	}
	static {  
		System.loadLibrary("SubstrateHook.cy");       
    }  
}
