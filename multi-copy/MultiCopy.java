import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.ArrayList;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.Timer;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.DefaultTableModel;

public class MultiCopy extends JFrame {
  private static final long serialVersionUID = 5908080586534669437L;
  EventHandler eh;
  Timer tScanner;
  JCheckBox jcb_enabled = new JCheckBox("Enable Autoflash");
  JTextField jt_file = new JTextField("");
  JButton jb_browse = new JButton("Browse");
  JFileChooser jfc = new JFileChooser();
  
  private int [] microbit_counter = new int[26];
  ActivityTable dtm_activity = new ActivityTable();
  JTable jt_activity = new JTable(dtm_activity);
  byte[] firmware;
  final String defaultFolder = "../microbit-binaries";
  
  public void loadFirmware(String fn) {
    File f = new File(fn);
    firmware = new byte[(int)(f.length())];
    DataInputStream dis;
    try {
      dis = new DataInputStream(new BufferedInputStream(new FileInputStream(f)));
      for (int i=0; i<firmware.length; i++) firmware[i]=dis.readByte();
      dis.close();
    } catch (Exception e) { e.printStackTrace(); } 
  }
  
  public void writeFirmware(File fn) {
    try {
      DataOutputStream dos = new DataOutputStream(new FileOutputStream(fn));
      for (int i=0; i<firmware.length; i++) dos.writeByte(firmware[i]);
      dos.close();
    } catch (Exception e) { e.printStackTrace(); }
  }
   
  
  class CopyThread extends Thread {
    File dest;
    CopyThread(File d) {
      dest = d;
    }
    
    public void run() {
      try {
        Thread.sleep(1000);
        writeFirmware(dest);
      } catch (Exception e) { e.printStackTrace(); }
    }
  }
  
  /* Drive/file handling stuff */
  
  boolean isMicrobit(File f) {
    boolean ok = false;
    File[] fs = f.listFiles();
    if (fs!=null) {
      if (fs.length==2) {
        if ((new File(f.getPath()+"DETAILS.TXT").exists()) &&
            new File(f.getPath()+"MICROBIT.HTM").exists()) {
              ok=true;
        }
      }
      if (fs.length==3) {
        if ((new File(f.getPath()+"DETAILS.TXT").exists()) &&
            (new File(f.getPath()+"HELP_FAQ.HTM").exists()) &&
            (new File(f.getPath()+"AUTO_RST.CFG").exists())) { 
              ok=true;
        }
      }
    }
    return ok;
  }
 
  
  
  void updateDriveLetters() {
    File[] roots = File.listRoots();
    boolean[] microbit_present = new boolean[26];
    for (int i=0; i<26; i++) microbit_present[i]=false;
    for(int i = 0; i < roots.length ; i++) {
      int ch_value = (int) roots[i].getPath().charAt(0)-65;
      if (isMicrobit(new File(roots[i].getPath()))) microbit_present[ch_value]=true;
    }
    for (int i=0; i<microbit_counter.length; i++) {
      char drive = (char) (65+i);
      if ((microbit_present[i]) && (microbit_counter[i]==0)) {
        dtm_activity.addStatus(drive+":", "Copying");
        microbit_counter[i]=1;
        File src = new File(jt_file.getText());
        File dest = new File(drive+":\\"+src.getName());
        if (jcb_enabled.isSelected()) new CopyThread(dest).start();
      } else if ((!microbit_present[i]) && (microbit_counter[i]==1)) {
        dtm_activity.setStatus(drive+":", "Rebooting...");
        microbit_counter[i]=2;
      } else if ((microbit_present[i]) && (microbit_counter[i]==2)) {
        dtm_activity.setStatus(drive+":", "Rebooted - ready to remove");
        microbit_counter[i]=3;
      } else if ((!microbit_present[i]) && (microbit_counter[i]==3)) {
        dtm_activity.removeDrive(drive+":");
        microbit_counter[i]=0;
      }
    }
  }
  
  public MultiCopy() {
    super();
    JOptionPane.showMessageDialog(null,  "Unplug all micro:bits, and click OK!");
    setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    setTitle("MultiCopy for micro:bit");
    setSize(640,540);
    setLayout(new FlowLayout());
    getContentPane().setLayout(new BoxLayout(getContentPane(), BoxLayout.PAGE_AXIS));
    JPanel jp_title = new JPanel(new FlowLayout(FlowLayout.CENTER));
    jp_title.add(jt_file);
    jt_file.setPreferredSize(new Dimension(300,25));
    jp_title.add(jb_browse);
    jp_title.add(jcb_enabled);
    jcb_enabled.setSelected(false);
    getContentPane().add(jp_title);
    JPanel jp_main = new JPanel(new FlowLayout(FlowLayout.CENTER));
    JScrollPane jsp = new JScrollPane(jt_activity);
    jsp.setPreferredSize(new Dimension(400,330));
    jt_activity.setRowHeight(30);
    jp_main.add(jsp);
    getContentPane().add(jp_main);
    pack();
    
    eh = new EventHandler();
    tScanner = new Timer(500,eh);
    tScanner.start();
    jb_browse.addActionListener(eh);
    jcb_enabled.addActionListener(eh);
    jfc.setDialogTitle("Choose a .hex file to flash: ");
    jfc.setFileSelectionMode(JFileChooser.FILES_ONLY);
    jfc.setAcceptAllFileFilterUsed(false);
    FileNameExtensionFilter filter = new FileNameExtensionFilter("Binary .hex files", "hex");
    jfc.addChoosableFileFilter(filter);
    jfc.setCurrentDirectory(new File(defaultFolder));
    setVisible(true);
  }
  
  public static void main(String[] args) {
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        new MultiCopy();
      }
    });
  }
  
  class ActivityTable extends DefaultTableModel {
    private static final long serialVersionUID = 5522510478441915789L;
    public ActivityTable() {
      super(new String[] {"Drive", "Status"}, 0);
    }
    
    ArrayList<String> data = new ArrayList<String>();
    
    public boolean isCellEditable(int row, int col) {
      return false;
    }
    
    public String getColumnName(int col) {
      if (col==0) return new String("Drive");
      else if (col==1) return new String("Status");
      else return null;
    }
    
    public int getRowCount() { return 10; }
    public int getColumnCount() { return 2; }

    public Object getValueAt(int row, int col) {
      if (data.size()>row) {
        return data.get(row).split("\t")[col];
      } else return "";
    }
    
    public void setStatus(String drive, String status) {
      for (int i=0; i<data.size(); i++) {
        if (data.get(i).split("\t")[0].equals(drive)) {
          data.set(i,drive+"\t"+status);
          i=data.size();
          fireTableDataChanged();
          jt_activity.repaint();
        }
      }
    }
    
    public void removeDrive(String drive) {
      for (int i=0; i<data.size(); i++) {
        if (data.get(i).split("\t")[0].equals(drive)) {
          data.remove(i);
          i=data.size();
          fireTableDataChanged();
          jt_activity.repaint();
        }
      }
    }
    public void addStatus(String drive, String status) {
      data.add(drive+"\t"+status);
      fireTableDataChanged();
      jt_activity.repaint();
    }
  }
  
  class EventHandler implements ActionListener {
    public void actionPerformed(ActionEvent e) {
      Object src = e.getSource();
      if (src==tScanner) {
        updateDriveLetters();
      } else if (src==jb_browse) {
        int result = jfc.showOpenDialog(MultiCopy.this);
        if (result == JFileChooser.APPROVE_OPTION) {
          jt_file.setText(jfc.getSelectedFile().getAbsolutePath());
        }
      } else if (src==jcb_enabled) {
        boolean sel = jcb_enabled.isSelected();
        if (!sel) {
          jb_browse.setEnabled(true);
          jt_file.setEnabled(true);
        } else {
          if (!new File(jt_file.getText()).exists()) {
            JOptionPane.showMessageDialog(MultiCopy.this, "Browse for a .hex file");
            jcb_enabled.setSelected(false);
          } else {
            MultiCopy.this.loadFirmware(jt_file.getText());
            jb_browse.setEnabled(false);
            jt_file.setEnabled(false);
          }
        }
      }
    }
  }
}
