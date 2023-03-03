import java.io.*;
import java.util.concurrent.*;

public class Level1 implements Runnable{
	private String input;
	private BufferedReader bufferedReader;
	private BufferedWriter writer_level1;
	private BufferedWriter writer_level2;
	private Semaphore semaphore;

	public Level1(String input, BufferedReader bufferedReader, BufferedWriter writer_level1, BufferedWriter writer_level2, Semaphore semaphore) {
		this.writer_level2 = writer_level2;
		this.bufferedReader = bufferedReader;
		this.writer_level1 = writer_level1;
		this.semaphore = semaphore;
		this.input = input;
	}

	public void run() {
		String line;
		try {
			/* atat timp cat avem comenzi de citit */
			while ((line = bufferedReader.readLine()) != null) {
				/* ne asiguram ca 2 thread-uri nu iau aceeasi comanda */
				synchronized (this) {
					String[] str = line.split(",");

					/* numarul de produse = numarul de thread-uri de nivel 2 */
					int nr_products = Integer.parseInt(str[1]);
					Thread[] t = new Thread[nr_products];

					FileReader file_level2 = new FileReader(this.input + "/order_products.txt");
					BufferedReader bufferedReader2 = new BufferedReader(file_level2);

					for (int i = 0; i < nr_products; i++) {
						t[i] = new Thread(new Level2(bufferedReader2, str[0], semaphore, writer_level2));
						t[i].start();
					}

					/* ne asiguram ca am terminat de procesat toate produsele comenzii */
					for (int i = 0; i < nr_products; i++)
						try {
							t[i].join();
						} catch (InterruptedException e) {
							e.printStackTrace();
						}

					/* setam comanda ca fiind shipped */
					String status = line + ",shipped\n";
					if(nr_products != 0)
						writer_level1.write(status);
				}
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
