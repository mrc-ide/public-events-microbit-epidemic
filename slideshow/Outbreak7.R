library(statnet)

args <- commandArgs(trailingOnly = TRUE)
outbreakdatafilename <- args[1]    ## name of output file.
screen.width <- as.numeric(args[2])
screen.height <- as.numeric(args[3])
col1 <- args[4]
col2 <- args[5]
col3 <- args[6]
fout <- args[7]

outbreakdatafilename <- "../data/498461975_325.csv"
screen.width <- 1366
screen.height <- 768
col1 <- "#000000"
col2 <- "#000000"
col3 <- "#707070"


#############################
## Visualise network settings

seedingshape <- c(50,4,3)            ## shapes for Seed, Node, Terminal)
categorycolours <- c(col1,col2,col3) ## Colours for categories (1,2,?)
recencysizes <- c(0.8,1.3)           ## sizes of icon for recency
recencytextsizes <- c(0.8,1.4)       ## sizes of text for recency

## load the data

all_data <- read.csv(outbreakdatafilename, stringsAsFactors = FALSE)
frame <- 0

# Work out terminals

# If n_contacts was zero...

all_data$Seeding[all_data$Event == 'I' & all_data$NoContacts == 0] <- 'X'

# If there's a recovery event, but no infections...

recoveries <- all_data$ID[all_data$Event=='R']

for (recovery in recoveries) {
  if (nrow(all_data[!is.na(all_data$Infectedby) &
                    all_data$Infectedby == recovery, ])==0) {
    all_data$Seeding[all_data$ID == recovery] <- 'X'
  }
}

data <- all_data

ties <- length(na.omit(data$Infectedby))
ties <- max(1,ties)

#################################
## individualdata
## first make a dataset of data about each case

individualdata <- data[,c("ID", "Seeding", "Recency", "Category",
                          "Infectedby")]
names(individualdata)<-c("ID", "Seeding", "Recency", "Category",
                         "IDInfector")

#################################
## AB (note, B infect A)
## now want a dataset which has the infector (B) and infectee (A)

AB <- merge(individualdata,individualdata[,c("ID", "Seeding",
                                          "Recency", "Category")],
                                          by.x=c("IDInfector"),
                                          by.y=c("ID"), all.x=TRUE)

names(AB)<-c("IDB","IDA", "SeedingA","RecencyA","CategoryA", "SeedingB",
             "RecencyB", "CategoryB")

#######################
## Create a "who infects who" matrix
## rows = infector
## cols = infected

n_AB <- max(AB[,1:2], na.rm = TRUE)
n_AB <- max(n_AB, 1)
AB_mat <- matrix(0, nrow = (1+n_AB), ncol = (1+n_AB))
colnames(AB_mat) <- 0:n_AB
rownames(AB_mat) <- 0:n_AB

for(i in 1:nrow(AB)){
  AB_mat[1 + AB[i,1], 1 + AB[i,2] ] <- 1
}

infect_index <- union( which(rowSums(AB_mat) > 0),
                       which(colSums(AB_mat) == 1))
infect_index <- sort(infect_index)
n_AB <- length(infect_index)
if (n_AB > 0) {
  AB_mat <- AB_mat[infect_index, infect_index]
  AB_net <- network(AB_mat, directed = TRUE)

  ##############################
  #### static network plot settings

  vertex.scaling <- 250 / ties
  vertex.scaling <- min(1, vertex.scaling)
  label.scaling <- 200 / ties
  label.scaling <- min(1, label.scaling)
  sorted<-individualdata[order(individualdata$ID),]
  sorted<-subset(sorted, (sorted$Seeding == "N" |
                          sorted$Seeding == "S") | sorted$Seeding == "X")

  seeding.shape <- rep(seedingshape[2], n_AB)
  seeding.shape[which(sorted[,2] == "S")] <- seedingshape[1]
  seeding.shape[which(sorted[,2] == "X")] <- seedingshape[3]

  category.col <- rep(categorycolours[3], n_AB)
  category.col[which(sorted[,4] == "1")] <- categorycolours[1]
  category.col[which(sorted[,4] == "2")] <- categorycolours[2]

  recency.size <- rep(recencysizes[1], n_AB)
  recency.size[which(sorted[,3] == "Recent")] <- recencysizes[2]
  recencytext.size <- rep(recencytextsizes[1], n_AB)
  recencytext.size[which(sorted[,3] == "Recent")] <- recencytextsizes[2]

  png(file=args[7], width = screen.width, height = screen.height)

  lay.mat <- rbind( c(1,1), c(2,3))
  layout(lay.mat, heights=c(1,0.1))
  par(mar = c(0, 0, 0, 0))

  set.seed(111)
  layout.FR <- c(niter = 10000,
     max.delta   = n_AB,
     area        = 1.7 * n_AB ^ 2,
     cool.exp    = 3,
     repulse.rad = 0.375 * n_AB ^ 3
    )
  layout.FR <- as.list(layout.FR)

  plot.network(AB_net, vertex.rot = 45,
    vertex.col = category.col,
    vertex.cex = 1.25 * vertex.scaling * recency.size,
    vertex.sides = seeding.shape,
    displaylabels = TRUE,
    boxed.labels = FALSE,
    label.cex = 1.00 * label.scaling * recencytext.size,
    edge.lwd = 0.005, edge.col = "black",
    label = network.vertex.names(AB_net),
    label.col = "darkgreen",
    arrowhead.cex = 0.75 * vertex.scaling,
    mode="fruchtermanreingold", layout.par = layout.FR)
  dev.off()
}
