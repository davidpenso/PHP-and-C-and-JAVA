import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;


public class scheduler {
	 static List<process> processList = new ArrayList<process>();
	 static List<process> queue = new ArrayList<process>();
	 static List<process> ioList = new ArrayList<process>();
	 static List<process> originalList = new ArrayList<process>();
	 
	 //for printing purposes calculations
	 public static double finish=0;
	 public static double ioUse=0;
	 public static double ioPerfect=0;
	 public static double CPUUse=0;
	 public static double CPUPerfect=0;
	 public static double done=0;
	 
	//static Scanner scanedRand;
	
	public static Scanner scanner(String input_file) {

		try {
			Scanner buffered_file = new Scanner(new BufferedReader(new FileReader(input_file)));
			return buffered_file;
		} 

		catch(Exception e) {
			System.out.println("Error in scanning file " + input_file);
		}

		return null; 
	}
	public static void reader(Scanner input){
		//number of processes
		
		int processNumber=input.nextInt();
		for(int j=0;j<processNumber;j++){
			process process = new process();
			input.next();
		
			process.setArrival(input.nextInt());
			process.setCPUBurst(input.nextInt());
			int dav=input.nextInt();
			process.setTotalCPUTime(dav);
			process.setCPUTimeRemaining(dav);
			process.setIOBurst(input.nextInt());
			
			input.next();
			processList.add(process);
			originalList.add(process);
		}
	
	
	}
	
	public static void sortArrival(List<process> pro){
		for(int i = 1; i < pro.size(); i++) {
		      for(int k = i; k > 0; k--) {
		    	  
		        if(pro.get(k).getArrival() < pro.get(k - 1).getArrival()) {
		        //swap list method set
		          process c = pro.get(k);
		          pro.set(k, pro.get(k - 1));
		          pro.set(k - 1, c);
		        }
		      }
		    }
		
	}
	public static void prioritySet(List<process> pro){
		// set priorities
		for(int i = 0; i < pro.size(); i++) {
		      pro.get(i).setPriority(i);
		    }
	}
	public static void sortPriority(List<process> pro){
		for(int i = 1; i < pro.size(); i++) {
		      for(int k = i; k > 0; k--) {
		    	  
		        if(pro.get(k).getPriority() < pro.get(k - 1).getPriority()) {
		        //swap list method set
		          process c = pro.get(k);
		          pro.set(k, pro.get(k - 1));
		          pro.set(k - 1, c);
		        }
		      }
		    }
	}
	public static void sortCPUTime(List<process> pro){
		for(int i = 1; i < pro.size(); i++) {
		      for(int k = i; k > 0; k--) {
		    	 
		        if(pro.get(k).getCPUTimeRemaining() < pro.get(k - 1).getCPUTimeRemaining()) {
		        //swap list method set
		          process c = pro.get(k);
		          pro.set(k, pro.get(k - 1));
		          pro.set(k - 1, c);
		        }
		      }
		    }
	}
		public static  process deque(){
		if(queue.size() > 0) {
		      process head = queue.get(0);
		      queue.remove(0);
		      return head;
		    } else {
		      return null;
		    }
	}
	 public static void enqueue(process process) {
		    queue.add(queue.size(), process);
		  }
	 public static void updateWaitTime(int timeCollapsed) {
		    for(int i = 0; i < queue.size(); i++) {
		      process process = queue.get(i);
		      process.setWaitTime(process.getWaitTime() + timeCollapsed);
		    }
		  }
	 
	 
	 public static  int randomOS(int base, Scanner sc) {
		    int num = sc.nextInt();
		    return (num % base) + 1;
		  }
	 public static  void addToIO(process process) {
		    process.setStatus(process.BLOCKED);
		    ioList.add(process);
		  }
	 public static void unblockProcesses(List<process> intermediateList) {
		    for(int i = 0; i < ioList.size(); i++) {
		      process process = ioList.get(i);
		      if(process.getCurrentIOBurst() == 0) {
		        intermediateList.add(process);
		        process.setStatus(process.READY);
		        ioList.remove(i);
		        i--;
		      }
		    }
		 }
	 public static void updateIOTime(int time) {
	
		    for(int i = 0; i < ioList.size(); i++) {
		      process process = ioList.get(i);
		      //get current io burst
		      if((process.getCurrentIOBurst() - time) < 0) {
		        process.setCurrentIOBurst(0);
		      } else {
		        process.setCurrentIOBurst(process.getCurrentIOBurst() - time);
		      }
		      process.setIOTime(process.getIOTime() + time);
		    }
		 }
	 public static void printInput() {
		    String str1 = "The original input was: " + processList.size() + " ";
		    String str2 = "The (sorted) input is:  " + processList.size() + " ";
		    for(int i = 0; i < processList.size(); i++) {
		      process proc = originalList.get(i);
		      str1 += "( " + proc.getArrival() + " " +
		          proc.getCPUBurst() + " " +
		          proc.getTotalCPUTime() + " " +
		          proc.getIOBurst() + " ) ";
		      proc = processList.get(i);
		      str2 += "( " + proc.getArrival() + " " +
		          proc.getCPUBurst() + " " +
		          proc.getTotalCPUTime() + " " +
		          proc.getIOBurst() + " ) ";
		    }
		    System.out.println(str1);
		    System.out.println(str2);
		    System.out.println();
		  }
	 public static void printProcess() {
		    for(int i = 0; i < processList.size(); i++) {
		      process proc = processList.get(i);
		      System.out.println("Process " + i + ":");
		      System.out.println("(A,B,C,IO) = (" + 
		          proc.getArrival() + ", " +
		          proc.getCPUBurst() + ", " +
		          proc.getTotalCPUTime() + ", " +
		          proc.getIOBurst() + ")");
		      System.out.println("Finishing time: " + proc.getFinishingTime());
		      System.out.println("Turnaround time: " + proc.getTurnAroundTime());
		      System.out.println("I/O time: " + proc.getIOTime());
		      System.out.println("Waiting time: " + proc.getWaitTime());
		      System.out.println();
		    }
		  }
	 public static void setFinishing(int finished){
		 finish=finished;
	 }
	 public static double getFinishing(){
		 return finish;
	 }
	 public static void setIOUsed(double ioUsed){
		 ioUse=ioUsed;
	 }
	 public static double getIOUsed(){
		 return ioUse;
	 }
	 public static void setIOPerfect(double ioPer){
		 ioPerfect=ioPer;
	 }
	 public static double getIOPerfect(){
		 return ioPerfect;
	 }
	 public static void setCPUUsed(double cpuUsed){
		 CPUUse=cpuUsed;
	 }
	 public static double getCPUUsed(){
		 return CPUUse;
	 }
	 public static void CPUplus(){
		 CPUUse++;
	 }
	 public static void IOplus(){
		 ioPerfect++;
	 }
	 public static void setCPUPerfect(double cpuPer){
		 CPUPerfect=cpuPer;
	 }
	 public static double getCPUPerfect(){
		 return CPUPerfect;
	 }
	 public static void done( ){
		 done++;
	 }
	 public static void printSummary() {
		    System.out.println("Finishing time: " + finish);
		    System.out.println("CPU Utilization: " + CPUUse/(CPUUse+CPUPerfect));
		    
		    System.out.println("I/O Utilization: " + ioUse/(ioUse+ioPerfect));
		    //System.out.println("This is done: "+done);
		    double ans= done/finish *100;
		    System.out.println("Throughput: " + ans + " processes per hundred cycles");
		    
		    
		    double turnAroundTime = 0;
		    for(int i = 0; i < processList.size(); i++) {
		      turnAroundTime =turnAroundTime+ processList.get(i).getTurnAroundTime();
		    }
		    
		    System.out.println("Average turnaround time: " + turnAroundTime/processList.size());
		    double waitTime = 0;
		    for(int i = 0; i < processList.size(); i++) {
		      waitTime =waitTime+ processList.get(i).getWaitTime();
		    }
		    
		    System.out.println("Average waiting time: " + waitTime/processList.size());
		  }

}
