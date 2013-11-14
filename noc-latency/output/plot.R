library (plyr)
library (ggplot2)
library (reshape2)

read_latency <- function (filename) {
    x <- do.call("rbind", lapply(filename, read.csv, header = TRUE, sep = ";"))
    x <- ddply (x, .(direction, size), summarise, time = mean (time))
    return (x)
}

plot_latency <- function (f, er, scale) {
    x <- read_latency (er)
    ggplot (x, aes (factor(size), time, group = direction, colour = factor(direction, labels=c("master-slave","slave-master")))) +
    theme_bw () +
    geom_line() +
    xlab ("Number of bytes") +
    ylab ("Time (in microseconds)") +
    guides(colour = guide_legend (nrow = 1)) +
    scale_y_continuous(breaks = c(scale)) +
    theme (legend.title = element_blank (),
           legend.position = "bottom",
           legend.direction = "vertical",
           axis.text.x = element_text(angle = 45, hjust = 1)) +
    ggsave (f)
}

#------------------------------------------------------------------------------

plot_latency ("channel-1-1.pdf", "channel-1-1.csv", seq(0, 14000, 1000))
#plot_latency ("channel-1-2.pdf", "channel-1-2.csv", seq(0, 28000, 2000))
#plot_latency ("channel-1-4.pdf", "channel-1-4.csv", seq(0, 56000, 4000))
#plot_latency ("channel-1-8.pdf", "channel-1-8.csv", seq(0, 112000, 8000))
#plot_latency ("channel-1-16.pdf", "channel-1-16.csv", seq(0, 224000, 16000))

plot_latency ("portal-1-1.pdf", "portal-1-1.csv", seq(0, 100000, 2000))
