/*
LangSupport.java is part of the micro:bit epidemic project.
It is part of the MicroEpiSlideshow code, and allows all
text in the slideshow to be looked up from an external 
file, thus making future multi-language translations of
the slideshow possible.

The MIT License (MIT)

Copyright (c) 2018 Wes Hinsley
MRC Centre for Global Infectious Disease Analysis
Department of Infectious Disease Epidemiology
Imperial College London

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

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
