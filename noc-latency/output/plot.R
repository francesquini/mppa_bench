library (plyr)
library (ggplot2)
library (reshape2)

read_latency <- function (er) {
    filenames <- list.files (path = "./", pattern = er, full.names = TRUE)
    x <- do.call ("rbind", lapply (filenames, read.csv, header = TRUE, sep = ";"))
    #x <- ddply (x, .(direction, size), summarise, time = mean (time))
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

plot_latency <- function (f, er, scale) {
    x <- read_latency (er)
    # inserts error column for obtaining an 95% conf. interval
    x <- ddply (x, .(direction, size), summarise, e = error (time, 0.95), time = mean (time))
    plot <- ggplot (x, aes (factor (size), time, group = direction, colour = direction, shape = direction)) +
    theme_bw () +
    geom_point() +
    geom_line () +
    # displays nice confidence interval
    geom_ribbon (aes (ymin = time + e, ymax = time - e, 
    				  fill = direction),
                 	  alpha = 0.2, linetype = 0) +
    guides(fill=FALSE) +
    xlab ("Number of bytes") +
    ylab ("Time (us)") +
    guides(colour = guide_legend (nrow = 1)) +
    theme (legend.title = element_blank (),
           legend.position = "bottom",
           legend.direction = "vertical",
           axis.text.x = element_text(angle = 45, hjust = 1))

	if ( !is.null(scale) )
		plot <- plot + scale_y_continuous(breaks = c(scale))

	plot
	
    ggsave (f)    
}

#------------------------------------------------------------------------------

#plot_latency ("plot.pdf", "data.csv", NULL)

plot_latency ("channel-1-1.pdf", "channel-1-1.csv", seq(0, 14000, 1000))
plot_latency ("channel-1-2.pdf", "channel-1-2.csv", seq(0, 28000, 2000))
plot_latency ("channel-1-4.pdf", "channel-1-4.csv", seq(0, 56000, 4000))
plot_latency ("channel-1-8.pdf", "channel-1-8.csv", seq(0, 112000, 8000))
plot_latency ("channel-1-16.pdf", "channel-1-16.csv", seq(0, 224000, 16000))

plot_latency ("portal-1-1.pdf", "portal-1-1.csv", seq(0, 3300, 300))
plot_latency ("portal-1-2.pdf", "portal-1-2.csv", seq(0, 6000, 600))
plot_latency ("portal-1-8.pdf", "portal-1-8.csv", seq(0, 12000, 900))
plot_latency ("portal-1-16.pdf", "portal-1-16.csv", seq(0, 24000, 1300))

plot_latency ("portal-async-1-1.pdf", "portal-async-1-1.csv", seq(0, 14000, 400))
plot_latency ("portal-async-1-2.pdf", "portal-async-1-2.csv", seq(0, 14000, 400))
plot_latency ("portal-async-1-4.pdf", "portal-async-1-4.csv", seq(0, 14000, 400))
plot_latency ("portal-async-1-8.pdf", "portal-async-1-8.csv", seq(0, 14000, 600))
plot_latency ("portal-async-1-16.pdf", "portal-async-1-16.csv", seq(0, 14000, 800))


file.remove("./Rplots.pdf")