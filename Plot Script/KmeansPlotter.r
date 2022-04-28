library(ggplot2)
library(hrbrthemes)

k = read.csv("kmeansCentroidData.csv")
ggplot(k, aes(x, y)) + geom_point() + xlim(0, 360) + ylim(-90, 90) + ggtitle("K-means plot") + xlab("x coordinate") + ylab("y coordinate") + hrbrthemes::theme_ft_rc()