// Language Support. Wes, Jan 2017
// First use: Barcode Epidemic V3, Wes, Jan 2017

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;

public class LangSupport {
  private ArrayList<String> supportedLanguages = new ArrayList<String>();
  private ArrayList<String> keywords = new ArrayList<String>();
  private ArrayList<ArrayList<String>> translations = new ArrayList<ArrayList<String>>(); // language, keyword
  private int selected_language=0;
  
  public void setLanguage(String s) {
    boolean found=false;
    for (int i=0; i<supportedLanguages.size(); i++) {
      if (supportedLanguages.get(i).equals(s)) {
        found=true;
        selected_language=i;
        i=supportedLanguages.size();
      }
    }
    if (!found) System.out.println("Error - language "+s+" not found in language file");
  }
  
  public int getKeywordIndex(String s) {
    boolean found=false;
    int result=-1;
    for (int i=0; i<keywords.size(); i++) {
      if (keywords.get(i).equals(s)) {
        found=true;
        result=i;
        i=keywords.size();
      }
    }
    if (!found) System.out.println("Error - keyword "+s+" not found in language file");
    return result;
  }
  
  public String getText(String s) {
    return translations.get(selected_language).get(getKeywordIndex(s));
  }
  
  public LangSupport() {
    try {
      // Parse twice for simplicity. Get language count first.
      BufferedReader br = new BufferedReader(new FileReader("lang.txt"));
      String s = br.readLine();
      String[] bits;
      while (s!=null) {
        s=s.trim();
        if (s.trim().length()>0) {
          if (!s.startsWith("#")) {
            bits=s.split(":");
            if (bits[0].equals("__")) {
              keywords.add(bits[1]);
            } else {
              boolean found=false;
              for (int j=0; j<supportedLanguages.size(); j++) {
                if (supportedLanguages.get(j).equals(bits[0])) {
                  found=true;
                  j=supportedLanguages.size();
                }
              }
              if (!found) supportedLanguages.add(bits[0]);
            }
          }
        }
        s=br.readLine();
      }
      br.close();
      for (int i=0; i<supportedLanguages.size(); i++) {
        ArrayList<String> t = new ArrayList<String>();
        for (int j=0; j<keywords.size(); j++) t.add("");
        translations.add(t);
      }
      
      br = new BufferedReader(new FileReader("lang.txt"));
      s = br.readLine();
      int key_index=-1;
      while (s!=null) {
        s=s.trim();
        if (s.length()>0) {
          if (!s.startsWith("#")) {
            bits=s.split(":");
            if (bits[0].equals("__")) {
              key_index=0;
              while (!bits[1].equals(keywords.get(key_index))) key_index++;
            } else {
              int lang_index=0;
              while (!bits[0].equals(supportedLanguages.get(lang_index))) lang_index++;
              
              translations.get(lang_index).set(key_index,bits[1]);
            }
          }
        }
        s=br.readLine();
      }
      br.close();
      
    } catch (Exception e) {
      System.out.println("Error reading languages");
      e.printStackTrace();
    }
    
  }
}
