library (plyr)
library (ggplot2)
library (reshape2)

read_latency <- function (er) {
    filenames <- list.files (path = "./", pattern = er, full.names = TRUE)
    x <- do.call ("rbind", lapply (filenames, read.csv, header = TRUE, sep = ";"))
    return (x)
}

# Computes confidence interval from t-student dist.
# - values: a list of numbers (samples)
# - percentage: the percent range of the confidence interval (default is 95%)
error <- function (values, percentage) {
    n <- length (values)
    # half of error for each side
    t <- 1.0 - ((1.0 - percentage) / 2)
    return (qt (t, df = n - 1) * sd (values) / sqrt (n))
}

plot_latency <- function (f, er, scale=NULL) {
    x <- read_latency (er)
    # inserts error column for obtaining an 95% conf. interval
    x <- ddply (x, .(processor, size), summarise, e = error (time, 0.95), time = mean (time))
    plot <- ggplot (x, aes (factor(size), time, group = processor, colour = processor, shape = processor)) +
    theme_bw () +
    geom_point() +
    geom_line () +
    # displays nice confidence interval
    geom_ribbon (aes (ymin = time + e, ymax = time - e, 
    				  fill = processor),
                 	  alpha = 0.2, linetype = 0) +
    guides(fill=FALSE) +
    xlab ("Size (KB)") +
    ylab ("Time (us)") +
    guides(colour = guide_legend (nrow = 1)) +
    theme (legend.title = element_blank (),
           legend.position = "bottom",
           legend.direction = "vertical",
           axis.text.x = element_text(angle = 90, hjust = 1))

	if ( !is.null(scale) )
		plot <- plot + scale_y_continuous(breaks = c(scale))
	
	#plot

    ggsave (plot, file=f, width=16, height=8)
}

#------------------------------------------------------------------------------

plot_latency ("data_mppa.pdf", "data_mppa.csv")
plot_latency ("data_x86.pdf", "data_x86.csv")

#file.remove("./Rplots.pdf")