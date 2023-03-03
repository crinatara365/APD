import java.io.*;
import java.util.concurrent.*;

public class Tema2{
	public static void main(String[] args)  throws IndexOutOfBoundsException{
		/* numarul maxim P de thread-uri */
		int nr_threads = Integer.parseInt(args[1]);

		/* fisierul de input */
		String input = args[0];

		Thread[] t = new Thread[nr_threads];
		Semaphore semaphore = new Semaphore(nr_threads);

		try {
			FileWriter writer1 = new FileWriter("orders_out.txt");
			BufferedWriter buffer1 = new BufferedWriter(writer1);
			FileWriter writer2 = new FileWriter("order_products_out.txt");
			BufferedWriter buffer2 = new BufferedWriter(writer2);
			BufferedReader bufferedReader = new BufferedReader(new FileReader(input+"/orders.txt"));

			for (int i = 0; i < nr_threads; i++) {
				t[i] = new Thread(new Level1(input, bufferedReader, buffer1, buffer2, semaphore));
				t[i].start();
			}

			for (int i = 0; i < nr_threads; i++) {
				try {
					t[i].join();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			buffer1.close();
			buffer2.close();
		}
		catch (IOException e) {
			e.printStackTrace();
		}
	}
}
