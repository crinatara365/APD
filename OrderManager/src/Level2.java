import java.io.*;
import java.util.concurrent.*;

public class Level2 implements Runnable{
	private BufferedReader bufferedReader;
	private String command;
	private Semaphore semaphore;
	private BufferedWriter writer;

	public Level2(BufferedReader bufferedReader, String command, Semaphore semaphore, BufferedWriter writer) {
		this.bufferedReader = bufferedReader;
		this.command = command;
		this.semaphore = semaphore;
		this.writer = writer;
	}
	public void run() {
		try {
			semaphore.acquire();
		}
		catch (InterruptedException e) {
			e.printStackTrace();
		}

		String line;
		try {
			while ((line = bufferedReader.readLine()) != null) {
				synchronized (this) {
					String[] str = line.split(",");

					/* daca gasim produsul aferent id-ului comenzii noastre, il marcam ca shipped */
					if (str[0].equals(command)) {
						String status = line + ",shipped\n";
						writer.write(status);
					}
				}
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
		semaphore.release();
	}
}
