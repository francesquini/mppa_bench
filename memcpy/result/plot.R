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

plot_latency <- function (f, er, yscale=NULL, xscale=NULL) {
    x <- read_latency (er)
    # inserts error column for obtaining an 95% conf. interval
    x <- ddply (x, .(processor, size), summarise, e = error (time / 1000, 0.95), time = mean (time / 1000))
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
    ylab ("Time (ms)") +
    guides(colour = guide_legend (nrow = 1)) +
    theme (legend.title = element_blank (),
           legend.position = "bottom",
           legend.direction = "vertical",
           axis.text.x = element_text(angle = 45, hjust = 1))

	if ( !is.null(yscale) )
		plot <- plot + scale_y_continuous(breaks = c(yscale))

	if ( !is.null(xscale) )
		plot <- plot + scale_x_discrete(breaks = c(xscale))
	
	#plot

    ggsave (plot, file=f, width=5, height=5)
}

#------------------------------------------------------------------------------

plot_latency ("data_mppa.pdf", "data_mppa.csv", yscale = seq(0, 300, 25), xscale = c(128, 256, 512, 1024, 2048, 4096))
plot_latency ("data_x86.pdf", "data_x86.csv")

#file.remove("./Rplots.pdf")