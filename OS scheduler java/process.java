
public class process {
	public final static int INACTIVE = 0;
	public final static int READY = 1;
	public final static int RUNNING = 2;
	public final static int BLOCKED = 3;
	public final static int TERMINATED = 4;
	public int cpuBurst = 0;
	public int curCPUBurst= 0;
	public int ioBurst = 0;
	public int currentIOBurst = 0;
	public int arrival = 0;
	public int finishingTime = 0;
	public int status = INACTIVE;
	public int priority = 0;
	public int turnAroundTime = 0;
	public int cpuTimeRemaining = 0;
	public int totalCPUTime = 0;
	public int ioTime = 0;
	public int waitTime = 0;
	
	//cpu burst set get
	protected void setCPUBurst(int cpuBurst) {
	    this.cpuBurst = cpuBurst;
	  }

	protected int getCPUBurst() {
	    return cpuBurst;
	  }
	  //Cpu Burst at the time
	protected void setCurCPUBurst(int curCPUBurst) {
		    this.curCPUBurst = curCPUBurst;
		  }

	protected void decreaseCurCPUBurst(int timeCollapsed) {
		    this.curCPUBurst -= timeCollapsed;
		  }
	

	protected int getCurCPUBurst() {
		    return curCPUBurst;
		  }
	protected void setIOBurst(int ioBurst) {
	    this.ioBurst = ioBurst;
	  }

	protected int getIOBurst() {
	    return ioBurst;
	  }
    protected void setCurrentIOBurst(int currentIOBurst) {
		    this.currentIOBurst = currentIOBurst;
		  }

	protected void decreaseCurrentIOBurst(int timeCollapsed) {
		    this.currentIOBurst -= timeCollapsed;
		  }

	protected int getCurrentIOBurst() {
		    return currentIOBurst;
		 }
	protected void setArrival(int arrival) {
	    this.arrival = arrival;
	  }

	  protected int getArrival() {
	    return arrival;
	  }

	  protected void setFinishingTime(int finishingTime) {
	    this.finishingTime = finishingTime;
	  }

	  protected int getFinishingTime() {
	    return finishingTime;
	  }
	  protected void setStatus(int status) {
		    this.status = status;
		  }
		  
	protected int getStatus() {
		    return status;
		  }

	protected void setPriority(int priority) {
		    this.priority = priority;
		  }

	protected int getPriority() {
		    return priority;
		  }

	protected void setCPUTimeRemaining(int cpuTimeRemaining) {
		    this.cpuTimeRemaining = cpuTimeRemaining;
		  }

	protected void decreaseCPURemaining(int timeCollapsed) {
		    this.cpuTimeRemaining -= timeCollapsed;
		  }

	protected int getCPUTimeRemaining() {
		    return cpuTimeRemaining;
		  }

	protected void setTurnAroundTime(int turnAroundTime) {
		    this.turnAroundTime = turnAroundTime;
		  }

	protected int getTurnAroundTime() {
		    return turnAroundTime;
		  }
   protected void setIOTime(int ioTime) {
	    this.ioTime = ioTime;
	  }

	protected void increaseIOTime(int timeCollapsed) {
	    this.ioTime += timeCollapsed;
	  }

	protected int getIOTime() {
	    return ioTime;
	  }

	protected void setTotalCPUTime(int totalCPUTime) {
	    this.totalCPUTime = totalCPUTime;
	  }
	protected int getTotalCPUTime() {
	    return totalCPUTime;
	  }
	protected int getWaitTime(){
		return waitTime;
	}
	protected void setWaitTime(int waitTime){
			this.waitTime=waitTime;
	}
	
}
