StartProcessCommand(v_Iterator begin,v_Iterator end):bg(false),autorec(false){
	processName=*begin;
	v_Iterator iter=begin+1;
	while(iter!=end){
		if (*iter=="&"){
			bg=true;
		}
		else if (*iter=="--autorecovery"){
			autorec=true;
		}
		else{
			parsedargs.push_back(*iter);
		}
		++iter;
	}
}

StartProcessCommand(processName_,std::vector<std::string> args,bool bg_,bool autorec_):parsedargs(args),bg(bg_),
autorec(autorec_),processName(processName_)
{
}

void StartProcessCommand::execute(){

//Use semaphore to make sure parent called first
	std::string semName=std::string("/sem")+processName;
	sem_t *sem = sem_open(semName, O_CREAT, 0644, 0); 

	int rc=fork();
        if (rc<0){
            fprintf(stderr,"Fork failed\n");
        }else if (rc==0){

        	sem_t *sem = sem_open(semName, 0);
        	sem_wait(sem);

        	int size=parsedargs.length();
            char *myargs[size+1];
            for (int i=0;i<size;i++){
            		myargs[i]=strdup(parsedargs[i]);
            }
            myargs[size]=NULL;
            execvp(strdup(processName),myargs);
        }
        else{
        	auto sp1=make_shared<Process>(processName,parsedargs,bg,autorec);
        	 Shell* s=Shell::instance()
        	 s->addProcess(rc,sp1);

        	 //Now child process can execute
        	sem_post(sem);
        	if (!bg){
            		CheckWait::waitForExit(rc);
            	}            
        }

}

void StartProcessCommand::waitForExit(int rc){
        bool exitedCorrectly=false;
        while(!exitedCorrectly){
            int status;
            int wc=waitpid(rc,&status,0);
            if (wc<0){
                throw std::runtime_error("Waitpid did not return properly");
            }
            if (WIFEXITED(status)){
                if (WEXITSTATUS(status)==0){
                    exitedCorrectly=true;
                }
                else{
                    throw std::runtime_error("Child did not exit successfully.");
                }
            }else if (WIFSIGNALED(status)){
                throw std::runtime_error("Child process terminated by signal "+WTERMSIG(status));
            }  
            //If none of above true, child was either stopped or continued. Wait for termination.
        }
    }


void StartProcessCommand::sig_handler(int sig){
    int pid = 0;
    int status;
    do{
        pid = waitpid(-1,&status,WNOHANG);
        if (pid != 0){
            
            Process* deadProcess;
            Shell *s = Shell::instance();
            deadProcess = s->getProcess(pid);
            
            if (deadProcess->isBg()){
                if (WIFEXITED(status)){
                    s->set_state("Exited");
                    
                    if (WEXITSTATUS(status)==0){
                        std::cout<<"Child "<<pid<<" exited successfully";
                    }
                    else{
                        std::cout<<"Child "<<pid<<" did not exit successfully.";
                    }
                }
                else if (WIFSIGNALED(status)){
                    int terminated_sig = WTERMSIG(status);
                    s->set_signal(terminated_sig);
                    std::cout<<"Child "<<pid<<" terminated by signal "<<terminated_sig;
                }
            
                if (s->isAutoRecovery()) {
                    StartProcessCommand RecCommand()
                }
            }
            
        }
        
        
    }while(pid!=0)
    
    
    
    
    
}
