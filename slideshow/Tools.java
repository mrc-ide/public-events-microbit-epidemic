import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.GregorianCalendar;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import javax.swing.JComboBox;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import javafx.scene.control.ColorPicker;
import javafx.scene.paint.Color;


public class Tools {
  
  public static String toHex(ColorPicker cp) {
    
    Color c = cp.getValue();
    int r = (int) (c.getRed()*255);
    int g = (int) (c.getGreen()*255);
    int b = (int) (c.getGreen()*255);
    return "#"+Integer.toHexString(r)+Integer.toHexString(g)+Integer.toHexString(b);
  }
  
  public static void fromHex(String h, ColorPicker cp) {
    cp.setValue(Color.web(h));
  }
  
  public static Element loadDocument(String file) {
    Element root = null;
    try {
      File f = new File(file);
      DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
      DocumentBuilder db = dbf.newDocumentBuilder();
      Document doc = db.parse(f);
      root=doc.getDocumentElement();
      root.normalize();
    } catch (Exception e) { e.printStackTrace(); }
    return root;
  }
  
  public static String getAttribute(Node parent, String attname)  {
    Node n = parent.getAttributes().getNamedItem(attname);
    if (n==null) return null;
    else return n.getTextContent();
  }
  
  public static void removeAttribute(Node parent, String attname)  {
    parent.getAttributes().removeNamedItem(attname);
    
  }

  public static int countChildren(Node parent) {
    int i=0;
    for (int j=0; j<parent.getChildNodes().getLength(); j++) {
      if (parent.getChildNodes().item(j).getNodeType()==Node.ELEMENT_NODE) i++;
    }
    return i;
  }
  
  public static int countChildren(Node parent,String tag) {
    int i=0;
    for (int j=0; j<parent.getChildNodes().getLength(); j++) {
      if (parent.getChildNodes().item(j).getNodeType()==Node.ELEMENT_NODE) {
        if (parent.getChildNodes().item(j).getNodeName().equals(tag)) i++;
      }
    }
    return i;
  }
  public static Node getChildNo(Node parent,String tag,int n) {
    int i=0;
    Node result=null;
    for (int j=0; j<parent.getChildNodes().getLength(); j++) {
      if (parent.getChildNodes().item(j).getNodeType()==Node.ELEMENT_NODE) {
        if (parent.getChildNodes().item(j).getNodeName().equals(tag)) {
          if (i==n) {
            result = parent.getChildNodes().item(j);
            j=parent.getChildNodes().getLength();
          }
          i++;
        }
      }
    }
    return result;
  }
  
  public static Node getChildNo(Node parent,int n) {
    int i=0;
    Node result=null;
    for (int j=0; j<parent.getChildNodes().getLength(); j++) {
      if (parent.getChildNodes().item(j).getNodeType()==Node.ELEMENT_NODE) {
        if (i==n) {
          result = parent.getChildNodes().item(j);
          j=parent.getChildNodes().getLength();
        }
        i++;
      }
    }
    return result;
  }
  
  public static Node getTagWhereAttr(Node parent, String tag, String attr, String attrValue) {
    Node resultNode = null;
    int count = countChildren(parent,tag);
    for (int i=0; i<count; i++) {
      Node n = getChildNo(parent,tag,i);
      if (n.getAttributes().getNamedItem(attr).getTextContent().equals(attrValue)) {
        resultNode=n;
        i=count;
      }
    }
    return resultNode;
  }
  
  /*
  public static Node[] getTagsWhereAttr(Node parent, String tag, String attr, String attrValue) {
    ArrayList<ArrayList<Node>> resultNode = getTagsWhereAttr2D(parent,tag,attr,new String[] {attrValue});
    Node[] nn = new Node[resultNode.get(0).size()];
    for (int i=0; i<resultNode.get(0).size(); i++) {
      nn[i]=resultNode.get(0).get(i);
    }
    return nn;
  }
  */
  
  public static ArrayList<ArrayList<Node>> getTagsWhereAttr2D(Node parent, String tag, String attr, String[] attrs) {
    ArrayList<ArrayList<Node>> result = new ArrayList<ArrayList<Node>>();
    for (int i=0; i<attrs.length; i++) result.add(new ArrayList<Node>());
    int count = countChildren(parent,tag);
    for (int i=0; i<count; i++) {
      Node n = getChildNo(parent,tag,i);
      String _attr = n.getAttributes().getNamedItem(attr).getTextContent();
      for (int j=0; j<attrs.length; j++) {
        if (_attr.equals(attrs[j])) result.get(j).add(n);
      }
    }
    return result;
  }
  
  public static Node getTag(Node root, String name) {
    NodeList nl = root.getChildNodes();
    Node result = null;
    for (int i=0; i<nl.getLength(); i++) {
      if (nl.item(i).getNodeName().equals(name)) {
        result = nl.item(i);
        i=nl.getLength();
      }
    }
    return result;
  }
  
  public static Element addTag(Node parent, String name) {
    Element e = parent.getOwnerDocument().createElement(name);
    parent.appendChild(e);
    return e;
  }
  
  public static void setAttribute(Node parent, String attrname, String attrvalue) {
    Attr a = parent.getOwnerDocument().createAttribute(attrname);
    a.setTextContent(attrvalue);
    parent.getAttributes().setNamedItem(a);
  }
  
  
  public static void selectJCB(String item, JComboBox<String> jcb) {
    for (int i=0; i<jcb.getItemCount(); i++) {
      if (jcb.getItemAt(i).toString().equals(item)) {
        jcb.setSelectedIndex(i);
        i=jcb.getItemCount();
      }
    }
  }
   

  // Download a URL
  
  public static void saveUrl(final String filename, final String urlString) throws MalformedURLException, IOException {
    BufferedInputStream in = null;
    FileOutputStream fout = null;
    try {
      URL url = new URL(urlString);
      getModDate(urlString);
      in = new BufferedInputStream(url.openStream());
      fout = new FileOutputStream(filename);
      final byte data[] = new byte[1024];
      int count;
      while ((count = in.read(data, 0, 1024)) != -1) {
          fout.write(data, 0, count);
      }
    } finally {
      if (in != null) {
        in.close();
      }
      if (fout != null) {
        fout.close();
      }
    }
  }
  
  public static void getModDate(final String urlString) throws MalformedURLException, IOException {
    try {
      URL url = new URL(urlString);
      HttpURLConnection urlc = (HttpURLConnection) url.openConnection();
      urlc.setRequestMethod("HEAD");
      urlc.getInputStream();
      int size = urlc.getContentLength();
      long dd = urlc.getLastModified();
      GregorianCalendar gc = new GregorianCalendar();
      gc.setTimeInMillis(dd);
      System.out.println(urlString+"\t"+size+"\t"+gc.get(GregorianCalendar.DATE)+"-"+(1+gc.get(GregorianCalendar.MONTH))+"-"+gc.get(GregorianCalendar.YEAR));
      urlc.disconnect();
    } catch (Exception e) {}
  }
  
  public static boolean extractFromZip(final String zipfile, String[] files, String output_path) {
    boolean success=false;
    try {
      ZipInputStream zis = new ZipInputStream(new FileInputStream(zipfile));
      ZipEntry entry;
      byte[] buffer = new byte[16384];
      boolean done=false;
      int done_files=0;
      while ((!done) && ((entry = zis.getNextEntry())!=null)) {
        for (int i=0; i<files.length; i++) {
          if (files[i]!=null) {
            if (files[i].equals(entry.getName())) {
              FileOutputStream output = new FileOutputStream(output_path+File.separator+entry.getName());
              int len=0;
              while ((len=zis.read(buffer))>0) output.write(buffer,0,len);
              output.close();
              files[i]=null;
              done_files++;
              if (done_files==files.length) {
                done=true;
                success=true;
              }
              i=files.length;
            }
          }
        }
      }
      zis.close();
      
    } catch (Exception e) {}
    return success;
  }
  
  
  public static Element newDocument(String roottag) {
    Element root=null;
    try {
      DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
      DocumentBuilder db = dbf.newDocumentBuilder();
      Document doc = db.newDocument();
      root = doc.createElement(roottag);
      doc.appendChild(root);
    } catch (Exception e) { e.printStackTrace(); }
    return root;
  }
  
  public static void writeXML(Element root,String file) {
    try {
      TransformerFactory tf = TransformerFactory.newInstance();
      tf.setAttribute("indent-number",2);
      Transformer t = tf.newTransformer();
      
      t.setOutputProperty(OutputKeys.INDENT,"yes");
      t.setOutputProperty(OutputKeys.METHOD,"xml");
      
      
      StreamResult res = new StreamResult(new OutputStreamWriter(new FileOutputStream(file),"utf-8"));
      DOMSource src = new DOMSource(root.getOwnerDocument());
      t.transform(src,res);
      
            
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  
}
