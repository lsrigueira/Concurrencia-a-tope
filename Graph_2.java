import java.awt.*;
import java.awt.font.*;
import java.awt.geom.*;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.time.LocalTime;
import java.util.ArrayList;
import java.util.Scanner;
import javax.swing.*;

public class Graph_2 extends JPanel {

	private static BufferedReader br;
	private static Scanner teclado;
	private static ArrayList<Integer> data = new ArrayList<Integer>();
	private static ArrayList<Long> valores = new ArrayList<Long>();
	public static int nodos = 0;
	public static String procesos = "";

	protected void paintComponent(Graphics g) {

		int PAD = data.size();
		super.paintComponent(g);
		Graphics2D g2 = (Graphics2D)g;
		Graphics2D g3 = (Graphics2D)g;
		g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				RenderingHints.VALUE_ANTIALIAS_ON);
		g3.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				RenderingHints.VALUE_ANTIALIAS_ON);
		int w = getWidth();
		int h = getHeight();
	//y
		g2.draw(new Line2D.Double(PAD+100, PAD, PAD+100, h-PAD-100));

	//x
		g2.draw(new Line2D.Double(PAD+100, h-PAD-100, w-PAD, h-PAD-100));

		Font font = g2.getFont();
		FontRenderContext frc = g2.getFontRenderContext();
		LineMetrics lm = font.getLineMetrics("0", frc);
		float sh = lm.getAscent() + lm.getDescent();

		String s = "TIEMPO EN SEGUNDOS";
		float sy = PAD + ((h - 2*PAD) - s.length()*sh)/2 + lm.getAscent();
		for(int i = 0; i < s.length(); i++) {
			String letter = String.valueOf(s.charAt(i));
			float sw = (float)font.getStringBounds(letter, frc).getWidth();
			float sx = (PAD - sw)/2;
			g2.drawString(letter, sx+15, sy-30);
			sy += sh;
		}
		s = "PROCESO DE ".concat(procesos.toUpperCase()).concat(" - PARA ").concat(String.valueOf(nodos)).concat(" NODOS");
		sy = h - PAD + (PAD - sh)/2 + lm.getAscent();
		float sw = (float)font.getStringBounds(s, frc).getWidth();
		float sx = (w - sw)/2;
		g2.drawString(s, sx, sy-35);


		double xInc = (double)(w - 2*PAD-150)/(data.size()-1);
		double scale = (double)(h - 2*PAD-150)/getMax(data);
		g2.setPaint(Color.green.darker());
		for(int i = 0; i < data.size()-1; i++) {
			double x1 = PAD+100 + i*xInc;
			double y1 = h - PAD-100 - scale*data.get(i);
			double x2 = PAD+100 + (i+1)*xInc;
			double y2 = h - PAD-100 - scale*data.get(i+1);
			g2.draw(new Line2D.Double(x1, y1, x2, y2));
		}

		for(int i = 0; i < data.size(); i++) {
			if (procesos == "anulacions") {
				g2.setPaint(Color.red);
			} else if (procesos == "pagos") {
				g2.setPaint(Color.darkGray);
			} else if (procesos == "pre-reservas") {
				g2.setPaint(Color.green.darker());
			} else if (procesos == "gradas") {
				g2.setPaint(Color.blue);
			} else if (procesos == "eventos") {
				g2.setPaint(Color.magenta.darker());
			}

			double x = PAD+100 + i*xInc;
			double y = h - PAD-100 - scale*data.get(i);
			g2.fill(new Ellipse2D.Double(x-8, y-8, 16, 16));

			String t = Integer.toString(i+1);
			for(int p = 0; p < t.length(); p++) {
				String letter = String.valueOf(t.charAt(p));
				float tx = (float) (PAD+100 + i*xInc);
				g2.drawString(letter, tx-3, 900);
				g2.draw(new Line2D.Double(tx,850, tx, 880));
}

			String f = Integer.toString(data.get(i));
			int posx = 39;
			float ty = (float) (h - PAD-100 - scale*data.get(i));
			g2.draw(new Line2D.Double(90, ty, 120, ty));
			for(int p = 0; p < f.length(); p++) {
				String letter = String.valueOf(f.charAt(p));
				g2.drawString(letter, posx, ty+5);
				posx += 10;
			}
		}
	}

	private int getMax(ArrayList<Integer> data) {
		int max = -Integer.MAX_VALUE;
		for(int i = 0; i < data.size(); i++) {
			if(data.get(i) > max)
				max = data.get(i);
		}
		return max;
	}

	public static Integer obtenerOpcion() {
		String option = null;
		if(teclado.hasNext()) {
			option = teclado.next();
		}
		if (option.matches("[0-9]+")) {
			return Integer.parseInt(option);
		} else {
			return -1;
		}
	}

	public static void main(String[] args) {
//				data.add(1);
//				data.add(3);
//				data.add(2);
//				data.add(6);
//				data.add(7);

		ArrayList <LocalTime> tiempos = new ArrayList<LocalTime>();
		ArrayList <String> ficheros = new ArrayList<String>();
		String directory = System.getProperty("user.dir")+"/";
		String nombre_fichero = "";
		Integer g,j,k, eleccion = 0;
		teclado = new Scanner(System.in);
		InputStreamReader input = new InputStreamReader(System.in);
		BufferedReader buffer = new BufferedReader(input);
		Scanner sc = new Scanner(System.in);

		System.out.println("\nIntroduzca el número de nodos a considerar:\n");
		nodos = obtenerOpcion();
		eleccion = nodos;
		System.out.println("\nNodos introducidos " + nodos);
		System.out.println("\nIntroduzca el tipo de proceso a comprobar:\n");
		procesos = sc.nextLine();

		for (j=0;j<eleccion;j++) {
			//System.out.println("\nIntroduzca el directorio del fichero log " + (j+1) + ":\n");
			//try {
				nombre_fichero = "".concat(String.valueOf(j+1)).concat(procesos).concat(".txt");
				System.out.println(nombre_fichero);
			//} catch (IOException e) {
			//	e.printStackTrace();
			//}
			ficheros.add(nombre_fichero);
		}
		try {
			int w = 0;
			for(k=0;k<ficheros.size();k++) {
				int dato = 0;
				tiempos = new ArrayList<LocalTime>();
				valores = new ArrayList<Long>();
				nombre_fichero = ficheros.get(k);
				File file = new File(directory+nombre_fichero);
				br = new BufferedReader(new FileReader(file));
				String st="";
				while ((st = br.readLine()) != null) {
					String[] parts = st.split("-");
					for (int i=0;i<parts.length;i++) {
						//System.out.println(parts[i]);
					}
					//System.out.println(parts.length);
					LocalTime hora = LocalTime.parse(parts[1].trim());
					tiempos.add(hora);
				}
				System.out.println("Tamaño: " + tiempos.size());
				for (g=0;g<(tiempos.size());g+=2) {
					long diferencia = (long) (tiempos.get(g+1).toNanoOfDay() - tiempos.get(g).toNanoOfDay())/1000000;
					valores.add(diferencia);
				}
					System.out.println("Tamaño valores: " + valores.size());
				long suma = 0, media = 0;
			//	int dato = 0;
				for (int x =0;x<valores.size();x++) {
					suma = suma + valores.get(x);
				}
				if (tiempos.size() != 0) {
					media = suma / valores.size();

					dato = (int) media;
					data.add(dato);
				} else {
					System.out.printf("1 Estoy aqui");
					data.add(0);
				}
			}
			for (int z =0;z<data.size();z++) {
				w++;
				System.out.println(w + ". " + data.get(z));
			}

		}catch (IOException ex) {
			ex.printStackTrace();

		}
		JFrame f = new JFrame();
		f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		f.add(new Graph_2());
		f.setSize(1800,1000);
		f.setLocation(50,200);
		f.setVisible(true);
	}
}
