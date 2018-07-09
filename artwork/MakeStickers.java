import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.io.File;

import javax.imageio.ImageIO;

public class MakeStickers {
  
  public static void argsMsg() {
    System.out.println("Example usage: java MakeStickers 0 99 sticker.png");
    System.exit(-1);
  }

  public static void main(String[] args) throws Exception {
    int start = 0;
    int end = 99;
    String sticker = "sticker.png";
    if (args.length==3) {
      try {
        start = Integer.parseInt(args[0]);
        end = Integer.parseInt(args[1]);
        sticker = args[2];
      } catch (Exception e) { argsMsg(); }
    } else if (args.length!=0) argsMsg();

    BufferedImage bi = new BufferedImage(4960, 7008, BufferedImage.TYPE_3BYTE_BGR);
    Graphics2D g = (Graphics2D) bi.getGraphics();
    g.setColor(Color.WHITE);
    g.fillRect(0, 0, 4960, 7008);
    g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
    g.setFont(new Font("Calibri",Font.BOLD, 192));
    BufferedImage back = ImageIO.read(new File(sticker));
    int page=1;
    FontMetrics fm = g.getFontMetrics();
    while (start<=end) {
      g.setColor(new Color(24,24,24));
      for (int i=0; i<3; i++) {
        for (int j=0; j<7; j++) {
          if (start<=end) {
            int left = (i*1602) + 280;
            int top = (j*927) + 366;
            g.drawImage(back, left, top, null);
            g.drawString(String.valueOf(start), ((left+529)-(fm.stringWidth(String.valueOf(start))/2)), top+220);
          }
          start++;
        }
      }
      ImageIO.write(bi, "PNG",  new File("page"+page+".png"));
      page++;
      g.setColor(Color.WHITE);
      g.fillRect(0, 0, 4960, 7008);
      
      
    }
    
  }

}
