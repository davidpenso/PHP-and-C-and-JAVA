import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;


public class rr {

	public static void main(String[] args) {
		scheduler schedule= new scheduler();

		Scanner file = scheduler.scanner(args[0]);
		scheduler.reader(file);
		Scanner scanedRand=scheduler.scanner(args[1]);
		
		scheduler.sortArrival(scheduler.processList);
		scheduler.prioritySet(scheduler.processList);
		
		rrAlgorithm(schedule, scanedRand);
		scheduler.printInput();
		scheduler.printProcess();
		scheduler.printSummary();
	}
	public static void rrAlgorithm(scheduler sc,Scanner scan){
		int quantum=2;
		//store list
		List<process> procList = new ArrayList<process>();
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
					procList.add(process);
					process.setStatus(process.READY);
				}
				
			}
			//remove from ioList unblock it
			if(scheduler.ioList.size()>0){
				scheduler.unblockProcesses(procList);
			}
			//sort the list by priority
			scheduler.sortPriority(procList);
			
			for(int i=0;i<procList.size();i++){
				scheduler.enqueue(procList.get(i));
			}
			//now that we put the process into the queue we can clear the temp process List
			procList.clear();
			
			//look at the cur process
			if(pro==null && scheduler.queue.size()>0){
				//System.out.println("Burdayim");
				pro =scheduler.deque();
				pro.setStatus(process.RUNNING);
				
				if(pro.getCurCPUBurst()==0){
					pro.setCurCPUBurst(scheduler.randomOS(pro.getCPUBurst(), scan));
					if(pro.getCurCPUBurst()>pro.getCPUTimeRemaining()){
					//System.out.println(" hep Burdayim");
						pro.setCurCPUBurst(pro.getCPUTimeRemaining());
					}
				}
			}
			int timePassed=1;
			time++;
			
			if(scheduler.ioList.size()>0){
				
				scheduler.updateIOTime(timePassed);
				scheduler.setIOUsed(scheduler.getIOUsed()+1);
			}
			else{
				scheduler.setIOPerfect(scheduler.getIOPerfect()+1);
			}
			
			scheduler.updateWaitTime(timePassed);
			
			if(pro!=null){
				//System.out.println("Burdayim kac kere"+ pro.getCPUTimeRemaining());
				pro.setCurCPUBurst(pro.getCurCPUBurst() - 1);
				pro.setCPUTimeRemaining(pro.getCPUTimeRemaining()-1);
				quantum=quantum-1;
				if(pro.getCPUTimeRemaining()==0){
					//System.out.println("Burdayim3");
					pro.setFinishingTime(time);
					pro.setTurnAroundTime(pro.getFinishingTime()-pro.getArrival());
					todo--;
					scheduler.done();
					pro.setStatus(pro.TERMINATED);
					
					pro=null;
					quantum=2;
					
				}
				else if(pro.getCurCPUBurst()==0){
					//System.out.println("Burdayim4");
					pro.setCurrentIOBurst(scheduler.randomOS(pro.getIOBurst(),scan));
					scheduler.ioList.add(pro);
					//System.out.println("io list: "+scheduler.ioList);
					pro=null;
					quantum=2;
					
				}
				//preemtion
				else if(quantum == 0) {
			          // Preempt the running process.
			          pro.setStatus(process.READY);
			          procList.add(pro);
			          pro = null;
			          quantum = 2;
			        }
				
				scheduler.setCPUUsed(scheduler.getCPUUsed()+1);
			}
			else{
				scheduler.setCPUPerfect(scheduler.getCPUPerfect()+1);
			}
		}
		
		scheduler.setFinishing(time);
	}
	


}
