
#include "routemanger.h"

RouteManger::RouteManger()
{
    inits_variables();
}

RouteManger::~RouteManger()
{
    fclose(log);
}

int RouteManger::setLevel(int level)
{
    current_level=level;
}

int RouteManger::inits_variables()
{

    for(int l=0;l<MAX_LEVEL;l++)
    {
       diff_id_list[l][0]=0;
       for(int k=1;k<MAX_DIFFER_LEN;k++)
          diff_id_list[l][k]=-1;
    }

   for(int r=0;r<MAX_ROUTE_NUM;r++)
       for(int i=0;i<MAX_ROUTE_LEN;i++ )
           for(int j=0;j<MAX_ROUTE_LEN;j++)
           Index_Sequence[r].index_table[i][j]=-1;

   valid_route_num=0;
   current_level=1;
   is_compare_finished=false;
   is_algorithm_improved=true;

   prun_bigger=0;
   prun_unvalid=0;
   prun_same=0;
   prun_trace=0;
   total_nodes=0;

   log=fopen("/home/tupei/log.txt","w");
   return 0;
}

int RouteManger::add_to_id_list(unsigned int id_num,int level)
{
    unsigned int len=diff_id_list[level][0];
    if(len==0)
    {
        diff_id_list[level][1]=id_num;
        diff_id_list[level][0]++;
        return 0;
    }

    for(int k=1;k<=len;k++)
        if(diff_id_list[level][k]==id_num)
            return 0;
    len+=1;
    diff_id_list[level][len]=id_num;
    diff_id_list[level][0]=len;

    return 0;

}

int RouteManger::read_data_files(char *filepath)
{
    FILE *file=fopen(filepath,"r");
    fscanf(file,"%d\n",&valid_route_num);
#ifdef ROUTE_DEBUG
   cout<<"valid_route_num="<<valid_route_num<<endl;
#endif

    if(valid_route_num<1)
       return -1;

    unsigned int ip_num,subnet_num,as_num;
    int hop_id,hop_num;

    char ip[20]={'\0'};
    char subnet[20]={'\0'};

    for(int i=0;i<valid_route_num;i++)
    {

       //file>>hop_num;
        fscanf(file,"%d\n",&hop_num);
#ifdef ROUTE_DEBUG
      cout<<"route_"<<i<<" "<<"hops:"<<hop_num<<endl;
#endif

       for(int j=0;j<hop_num;j++)
       {
           for(int m=0;m<20;m++)
           {
               ip[m]='\0';
               subnet[m]='\0';
           }


           fscanf(file,"%d  %s  %s  %d\n",&hop_id,ip,subnet,&as_num);
          ip_num=ip_to_int(ip);
          subnet_num=ip_to_int(subnet);

#ifdef ROUTE_DEBUG
         cout<<ip_num<<" "<<subnet_num<<" "<<as_num<<endl;
#endif

          add_to_id_list(ip_num,1);
          add_to_id_list(subnet_num,2);
          add_to_id_list(as_num,3);

          all_routes[i].route[j].ID[IP_LEVEL]=ip_num;
          all_routes[i].route[j].ID[SUB_LEVEL]=subnet_num;
          all_routes[i].route[j].ID[AS_LEVEL]=as_num;
       }

       all_routes[i].valid_route_len=hop_num;

    }
    return 0;
}

unsigned int RouteManger:: ip_to_int(char *ip_addr)
{
    unsigned int result=0;
    unsigned int tmp=0;
    int shit=24;
    char *pstart=ip_addr;
    char *pend=ip_addr;

    while(*pend!='\0')
    {
        while(*pend!='.'&&*pend!='\0')
           pend++;

        tmp=0;

        while(pstart<pend)
        {
           tmp=tmp*10+(*pstart-'0');
           pstart++;
         }

         result+=(tmp<<shit);
         shit-=8;

        if(*pend=='\0')
             break;

         pstart=pend+1;
         pend++;
    }

    return result;
}

int RouteManger::int_to_ip(unsigned int ip,unsigned char*ip_addr)
{
    unsigned int tmp=ip;
    ip_addr[0]=(unsigned char)*((char*)&tmp+0);
    ip_addr[1]=(unsigned char)*((char*)&tmp+1);
    ip_addr[2]=(unsigned char)*((char*)&tmp+2);
    ip_addr[3]=(unsigned char)*((char*)&tmp+3);
     return 0;
}

int RouteManger::construct_index_sequence()
{
    int preIndex=-1;
    unsigned  int id;
    for(int r=0;r<valid_route_num;r++)
    {
        for(unsigned int i=1;i<=diff_id_list[current_level][0];i++)
        {
            preIndex=-1;
            for(int f=all_routes[r].valid_route_len-1;f>=0;f--)
            {
                id=all_routes[r].route[f].ID[current_level];
                if(id==diff_id_list[current_level][i])
                {
                    Index_Sequence[r].index_table[i-1][f]=f;
                    if(preIndex!=-1)
                        Index_Sequence[r].index_table[i-1][f+1]=-1;
                    preIndex=f;
                }
                else
                    Index_Sequence[r].index_table[i-1][f]=preIndex;
            }
         }
    }

    return 0;
}

int RouteManger::construct_pairs_tree()
{

    PairNode VirtualPair;
    VirtualPair.parent=-1;
    VirtualPair.index=0;
    VirtualPair.key=0;
    VirtualPair.level=0;
    VirtualPair.state='v';
    for(int i=0;i<valid_route_num;i++)
        VirtualPair.index_in_seuqences[i]=-1;

    Pairs_table[0]=VirtualPair;
    max_level=0;
    level[0]=0;

    for(int lev=0;lev<=max_level;lev++)
    {
        level_tree_generate(lev);
     }

    printf("Level: %d\n",current_level);
    printf("Total: %d\n",total_nodes);
    printf("Prune Unvalid Nodes: %d     %.3f\n",prun_unvalid,float(prun_unvalid)/total_nodes);
    printf("Prune Bigger   Nodes: %d     %.3f\n",prun_bigger,float(prun_bigger)/ total_nodes);
    printf("Prune  Parent-Same Nodes: %d     %.3f\n",prun_same,float(prun_same)/total_nodes);
    printf("Prune  Traces-Same  Nodes: %d     %.3f\n",prun_trace,float(prun_trace)/total_nodes);
    printf("-----------------------------------------------------\n");
    return 0;
}

int RouteManger::pair_index_compare(PairNode p,PairNode q)
{

    int count_p=0,count_q=0;

    for(int i=0;i<valid_route_num;i++)
    {
        if(p.index_in_seuqences[i]>=q.index_in_seuqences[i])
        {
            count_p++;
            if(count_p==valid_route_num)
                return 1;
        }

        if(p.index_in_seuqences[i]<=q.index_in_seuqences[i])
        {
            count_q++;
            if(count_q==valid_route_num)
                return -1;
        }
    }

    return 0;
}

int RouteManger::trace_back_lcs()
{
    if(max_level<1) return -1;
    int high=level[max_level];
    int low=level[max_level-1]+1;
    int index;
    int common_num=0;
    int key;
    unsigned char ip_addr[15];

    for(int i=high;i>=low;i--)
    {     
        index=Pairs_table[i].index;
        cout<<"MLCS:    ";
        while(Pairs_table[index].parent!=-1)
        {
            key=Pairs_table[index].key;
            if (current_level<=2)
            {
               int_to_ip(key,ip_addr);
               printf("%u.%u.%u.%u--",ip_addr[3],ip_addr[2],ip_addr[1],ip_addr[0]);
            }
            else
                printf("%d--",key);


            index=Pairs_table[index].parent;
        }
        cout<<endl;
    }
    return 0;
}

int  RouteManger::level_tree_generate(int lev)
{

    int high;
    int low;
    if(lev==0) high=low=0;
    else
    {
        high=level[lev];
        low=level[lev-1]+1;
    }

    int child_start_pos=high+1;
    int child_pos=child_start_pos;

    bool is_pair_valid=true;
    bool is_first_child=true;
    max_level=lev;
    PairNode pair;

    for(int l=low;l<=high;l++)
    {
        if (Pairs_table[l].state!='v')
               continue;

        for(unsigned int c=1;c<=diff_id_list[current_level][0];c++)
        {
            total_nodes++;
            is_pair_valid=true;
            int key=diff_id_list[current_level][c];


            if (is_algorithm_improved==true)                                //Setting Improved options
            {
            if (low!=0)
            {
                 if(Pairs_table[l].key==key)                            //prune the node with the same key
                 {
                   prun_same++;
                   continue;
                }

                 if(find_previous(key,level[lev])==0)           //prune the node with the same key on the traceback path
              {
                  prun_trace++;
                  continue;
              }
            }
            }

            for(int s=0;s<valid_route_num;s++)
            {
                int index=Pairs_table[l].index_in_seuqences[s]+1;
                int value=Index_Sequence[s].index_table[c-1][index];

                     if(value==-1)
                   {
                         if(is_algorithm_improved==true)                  //prune the node unvalid
                                 prun_unvalid++;

                         is_pair_valid=false;
                         break;
                    }

                pair.index_in_seuqences[s]=value;
            }

            if(is_pair_valid==false)
            {
                continue;
            }



            pair.key=diff_id_list[current_level][c];
            pair.level=lev+1;
            pair.state='v';
            pair.index=child_pos;
            pair.parent=Pairs_table[l].index;
            if(is_first_child)
            {
                for(int k=0;k<valid_route_num;k++)
                    Pairs_table[child_pos].index_in_seuqences[k]=pair.index_in_seuqences[k];
                Pairs_table[child_pos].key=pair.key;
                Pairs_table[child_pos].level=pair.level;
                Pairs_table[child_pos].parent=pair.parent;
                Pairs_table[child_pos].index=child_pos;
                Pairs_table[child_pos].state='v';
                child_pos++;
                is_first_child=false;
                continue;
            }

            level_tree_prunning(child_start_pos,child_pos,pair);

        }

    }

//printf("Level: %d  inter: [%d----%d)\n",lev+1,child_start_pos,child_pos);

    if(child_pos==child_start_pos)
    {

        max_level=lev;
        trace_back_lcs();
        return 0;
    }
    else
    {
        max_level=lev+1;
        level[max_level]=(child_pos-1);

    }

    return 1;
}

int RouteManger::pair_testify(PairNode & p,unsigned int key)
{

    int index=p.index;
    if(key!=p.key)
    {

        while(Pairs_table[index].parent!=-1)
       {
            index=Pairs_table[index].parent;
            if (key==Pairs_table[index].key)
                return 0;
        }
    }

    return 1;
}

int RouteManger::level_tree_prunning(int &child_start,int& child_end,PairNode &pair)
{
    int equal_count=0;
    int valid_pairs_num=0;

    for(int i=child_start;i<child_end;i++)
         if (Pairs_table[i].parent==pair.parent)
                valid_pairs_num++;

    for(int i=child_start;i<child_end;i++)
    {
        if(Pairs_table[i].state!='v')
            continue;
        if (Pairs_table[i].parent!=pair.parent)
            continue;

        int result=pair_index_compare(pair,Pairs_table[i]);

        if(result==0)
        {
            equal_count++;
            if(equal_count==valid_pairs_num)
            {
                for(int k=0;k<valid_route_num;k++)
                    Pairs_table[child_end].index_in_seuqences[k]=pair.index_in_seuqences[k];
                Pairs_table[child_end].key=pair.key;
                Pairs_table[child_end].level=pair.level;
                Pairs_table[child_end].parent=pair.parent;
                Pairs_table[child_end].index=child_end;
                Pairs_table[child_end].state='v';
                child_end++;
                break;
            }

            continue;
        }

        else if(result==-1)
        {
            Pairs_table[i].state='u';

        }

        else if(result==1)
        {
            prun_bigger++;                                               //prune nodes with bigger values on the same level
            break;
        }

        else
            return -1;
    }
    return 0;  
}

int RouteManger::clear()
{
    current_level=1;
    is_compare_finished=false;
    is_algorithm_improved=true;

    prun_bigger=0;
    prun_unvalid=0;
    prun_same=0;
    prun_trace=0;
    total_nodes=0;

    for(int r=0;r<MAX_ROUTE_NUM;r++)
    {
        all_routes[r].valid_route_len=0;
        for(int n=0;n<MAX_ROUTE_LEN;n++)
        {
            all_routes[r].route[n].ID[IP_LEVEL]=-1;
            all_routes[r].route[n].ID[SUB_LEVEL]=-1;
            all_routes[r].route[n].ID[AS_LEVEL]=-1;
        }
    }
    return 0;
}

int RouteManger::find_previous(int key,int pos)
{
    for(int i=1;i<=pos;i++)
        if (Pairs_table[i].state=='v' &&Pairs_table[i].key==key)
            return 0;
    return -1;
}
