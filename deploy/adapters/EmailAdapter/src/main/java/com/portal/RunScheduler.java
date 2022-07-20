package com.portal;

import java.util.Date;
import java.util.TimerTask;

import org.apache.log4j.Logger;
import org.springframework.batch.core.Job;
import org.springframework.batch.core.JobExecution;
import org.springframework.batch.core.JobParameters;
import org.springframework.batch.core.JobParametersBuilder;
import org.springframework.batch.core.launch.JobLauncher;
import org.springframework.beans.factory.InitializingBean;

public class RunScheduler extends TimerTask implements InitializingBean{
	private static final Logger logger = Logger.getLogger(RunScheduler.class);
	
	private JobLauncher jobLauncher;
	private Job job;
		
	public JobLauncher getJobLauncher() {
		return jobLauncher;
	}

	public void setJobLauncher(JobLauncher jobLauncher) {
		this.jobLauncher = jobLauncher;
	}

	public Job getJob() {
		return job;
	}

	public void setJob(Job job) {
		this.job = job;
	}

	@Override
	public void afterPropertiesSet() throws Exception {
	}

	@Override
	public void run() {
		try {
			 
			String dateParam = new Date().toString();
			JobParameters param = new JobParametersBuilder().addString("date", dateParam).toJobParameters();
		 
			logger.debug(dateParam);
			JobExecution execution = jobLauncher.run(job, param);
			logger.info("Exit Status : " + execution.getStatus());
		 
		    } catch (Exception e) {
		    	logger.error(e.getMessage());
		    }
	} 
}
