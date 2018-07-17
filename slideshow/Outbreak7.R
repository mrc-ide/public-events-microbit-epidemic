library(statnet)
##### inputting the data
## give the name of the	file with the outbreak data in it

args <- commandArgs(trailingOnly = TRUE)
outbreakdatafilename <-	args[1]	    ## name of output file.
screen.width <- as.numeric(args[2])
screen.height <- as.numeric(args[3])
col1 <- args[4]
col2 <- args[5]
col3 <- args[6]

# seeding variable: "S"	(seed),	"N" (reported node) ,"X" (known	terminal) , determines node shape
# recency variable: "Old", "New" determines node size
# category variable: 0 = unknown, 1 = category1, 2 = category2 - whatever that category means. Colour.

## load	the data

data <-	read.csv(outbreakdatafilename)

## check that it has loaded correctly and there	are the	number
## of cases you're expecting

#head(data)    ## check	how the	data looks
#summary(data) ## gives	characteristics	of the data in each column
ties<-length(na.omit(data$Infectedby))
ties<-max(1,ties)

## replaces typos of more than4	digits with shorter values (prevents memory problems with adjacency matrix)
substitute<-min(c(data$ID,1000), na.rm=TRUE)-1
data$ID[(data$ID>9999)]<-substitute

 
#################################
## individualdata
## first make a	dataset	of data	about each case

individualdata <- data[,c("ID",	"Seeding", "Recency", "Category","Infectedby")]
names(individualdata)<-c("ID", "Seeding", "Recency", "Category", "IDInfector")
#head(individualdata)

#################################
## AB (note, B infect A)
## now want a dataset which has	the infector (B) and infectee (A)

AB <- merge(individualdata,individualdata[,c("ID", "Seeding", "Recency", "Category")],
	by.x=c("IDInfector"),
	by.y=c("ID"), all.x=TRUE)
names(AB)<-c("IDB","IDA", "SeedingA","RecencyA","CategoryA", "SeedingB", "RecencyB", "CategoryB")
#head(AB)	## check what the data looks like


########################
## 2. NETWORK	      ##
########################

#######################
## Visualise network settings
seedingshape<-c(50,4,3)			## shape for seeding: S(Seed), N (node), X (terminal)	
categorycolours<-c(col1,col2,col3) ## Colours for categories (cat1, cat2, unknown)
recencysizes<-c(0.8,1.3)                ## sizes of icon for recency
recencytextsizes<-c(0.8,1.4)            ## sizes of text for recency

 

#######################
## Create a "who infects who" matrix
## rows	= infector
## cols	= infected
n_AB <-	max(AB[,1:2], na.rm=TRUE)
AB_mat <- matrix( 0, nrow=n_AB,	ncol=n_AB)
colnames(AB_mat) <- 1:n_AB
rownames(AB_mat) <- 1:n_AB
for(i in 1:nrow(AB)){
	AB_mat[	AB[i,1], AB[i,2] ] <- 1
}
infect_index <-	union( which(rowSums(AB_mat)>0), which(colSums(AB_mat)==1)  ) 
infect_index <-	sort(infect_index)
n_AB <-	length(infect_index)
AB_mat <- AB_mat[infect_index,infect_index]
AB_net <- network(AB_mat, directed=TRUE)

				
##############################
#### static network plot settings

vertex.scaling<-250/ties
vertex.scaling<-min(1,vertex.scaling)
label.scaling<-200/ties
label.scaling<-min(1,label.scaling)
sorted<-individualdata[order(individualdata$ID),]
sorted<-subset(sorted,(sorted$Seeding=="N" | sorted$Seeding=="S") | sorted$Seeding=="X")
seeding.shape <- rep(seedingshape[2], n_AB)
seeding.shape[which(sorted[,2]=="S")] <- seedingshape[1]
seeding.shape[which(sorted[,2]=="X")] <- seedingshape[3]

category.col<-rep(categorycolours[3], n_AB)
category.col[which(sorted[,4]=="1")] <-	categorycolours[1]
category.col[which(sorted[,4]=="2")] <-	categorycolours[2]

recency.size<-rep(recencysizes[1],n_AB)
recency.size[which(sorted[,3]=="New")] <- recencysizes[2]
recencytext.size<-rep(recencytextsizes[1],n_AB)
recencytext.size[which(sorted[,3]=="New")] <- recencytextsizes[2]

png(file='staticnetworkplot.png',width=screen.width,height=screen.height)
lay.mat	<- rbind( c(1,1),
		  c(2,3) )
layout(lay.mat,	heights=c(1,0.1))
#layout.show(3)
par(mar=c(0,0,0,0))	## reset margins for network plot
set.seed(111)
layout.FR <- c(niter	   = 10000,
	       max.delta   = n_AB,
	       area	   = 1.7*n_AB^2,
	       cool.exp	   = 3,
	       repulse.rad = 0.375*n_AB^3
	      )	    
layout.FR <- as.list(layout.FR)
plot.network(AB_net, vertex.rot=45,
	vertex.col=category.col, vertex.cex=1.25*vertex.scaling*recency.size,
	vertex.sides=seeding.shape,
	displaylabels=TRUE, boxed.labels=FALSE,	label.cex=1.25*label.scaling*recencytext.size,
	edge.lwd=0.005,	edge.col="black",
	label=network.vertex.names(AB_net),
	label.col="darkgreen",
	arrowhead.cex=0.75*vertex.scaling,
	mode="fruchtermanreingold", layout.par=layout.FR)

dev.off()




