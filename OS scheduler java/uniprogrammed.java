import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;


public class uniprogrammed {

	public static void main(String[] args) {
		scheduler schedule= new scheduler();

		Scanner file = scheduler.scanner(args[0]);
		scheduler.reader(file);
		Scanner scanedRand=scheduler.scanner(args[1]);
		
		scheduler.sortArrival(scheduler.processList);
		scheduler.prioritySet(scheduler.processList);
		uniprogrammedAlgorithm(schedule, scanedRand);
		scheduler.printInput();
		scheduler.printProcess();
		scheduler.printSummary();
	}
	public static void uniprogrammedAlgorithm(scheduler sc,Scanner scan){

		
		
		//actual process list from scheduler
		List<process> proc= sc.processList;
		int todo= proc.size();
		int time=0;
		// process variable
		process pro =null;
		
		while(todo>0){
			
			for(int j=0;j<proc.size();j++){
				process process = proc.get(j);
				if(process.getArrival()<=time && process.getStatus()== process.INACTIVE){
					scheduler.enqueue(process);
					process.setStatus(process.READY);
				}
				
			}
			if(pro==null){
				pro=scheduler.deque();
			}
			
			
			//look at the cur process
			if(pro.getStatus()==process.READY){
		
				pro.setCurCPUBurst(scheduler.randomOS(pro.getCPUBurst(), scan));
				if(pro.getCurCPUBurst()>pro.getCPUTimeRemaining()){
					pro.setCurCPUBurst(pro.getCPUTimeRemaining());
				}
				pro.setStatus(process.RUNNING);
			}
			int timePassed=1;
			time++;
			
			
			if(pro.getStatus()==process.RUNNING){
				scheduler.setCPUUsed(scheduler.getCPUUsed()+1);
				scheduler.setIOPerfect(scheduler.getIOPerfect()+1);
				pro.setCurCPUBurst(pro.getCurCPUBurst() - 1);
				pro.setCPUTimeRemaining(pro.getCPUTimeRemaining()-1);
				if(pro.getCPUTimeRemaining()==0){
					//System.out.println("Burdayim3");
					pro.setFinishingTime(time);
					pro.setTurnAroundTime(pro.getFinishingTime()-pro.getArrival());
					todo--;
					scheduler.done();
					pro.setStatus(pro.TERMINATED);
					pro=null;
				}
				else if(pro.getCurCPUBurst()==0){
					pro.setCurrentIOBurst(scheduler.randomOS(pro.getIOBurst(),scan));
					pro.setStatus(process.BLOCKED);
					
				}
				
			}
			else if(pro.getStatus()==process.BLOCKED){
				scheduler.setCPUPerfect(scheduler.getCPUPerfect()+1);
				scheduler.setIOUsed(scheduler.getIOUsed()+1);
				pro.setCurrentIOBurst(pro.getCurrentIOBurst()-1);
				pro.setIOTime(pro.getIOTime()+1);
				if(pro.getCurrentIOBurst()==0){
					pro.setStatus(process.READY);
				}
			}
			
			scheduler.updateWaitTime(timePassed);
		}
		
		scheduler.setFinishing(time);
	}
	

}
