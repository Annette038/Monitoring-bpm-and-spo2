# Se carga la librería a emplear
library(tidyverse)

# Se leen los datos de las personas (en formato .csv)
data_list <- lapply(1:20, function(i) {
  read.csv(paste0("person", i, ".csv"))
})

# Se combinan los datos de todas las personas en una misma combinación
# Se convierte orden los archivos en orden numérico para su posterior muestra en los gráficos
combined_data$Person <- factor(combined_data$Person, levels = as.character(1:20))

# Reformulación de la información
data_long <- combined_data %>%
  pivot_longer(cols = -c(Mediciones, Person), names_to = c(".value", "Device"), names_sep = "\\.") %>%
  mutate(Device = case_when(
    str_detect(Device, "Com$") ~ "InhalaCare",
    str_detect(Device, "Com2$") ~ "Huawei GT2",
    str_detect(Device, "Nue$") ~ "TirTir"
  ))

# Línea para checar estructura
str(data_long)

# Comienza código de visualización (gráficas)

# Boxplot para SpO2
ggplot(data_long, aes(x = Device, y = SpO2, fill = Device)) +
  geom_boxplot() +
  stat_summary(fun = mean, geom = "point", shape = 21, size = 3, fill = "yellow", color = "black") +  # Add mean points
  labs(title = "Comparación de valores de SpO2 en los dispositivos", 
       x = "Dispositivo", 
       y = "SpO2 (%)",
       fill = "Dispositivo") +
  theme_minimal() +
  theme(
    plot.title = element_text(size = 18),        # Increase title size
    axis.title.x = element_text(size = 14),      # Increase x-axis title size
    axis.title.y = element_text(size = 14),      # Increase y-axis title size
    axis.text.x = element_text(size = 11),       # Increase x-axis text (tick labels) size
    axis.text.y = element_text(size = 11),       # Increase y-axis text (tick labels) size
    legend.title = element_text(size = 14)       # Increase legend title size
  )

# Boxplot para LPM
ggplot(data_long, aes(x = Device, y = BPM, fill = Device)) +
  geom_boxplot() +
  stat_summary(fun = mean, geom = "point", shape = 21, size = 3, fill = "yellow", color = "black") +  # Add mean points
  labs(title = "Comparación de valores de latidos por minuto en los dispositivos", 
       x = "Dispositivo", 
       y = "Latidos por minuto (LPM)",
       fill = "Dispositivo") +
  theme_minimal() +
  theme(
    plot.title = element_text(size = 18),        # Increase title size
    axis.title.x = element_text(size = 14),      # Increase x-axis title size
    axis.title.y = element_text(size = 14),      # Increase y-axis title size
    axis.text.x = element_text(size = 11),       # Increase x-axis text (tick labels) size
    axis.text.y = element_text(size = 11),       # Increase y-axis text (tick labels) size
    legend.title = element_text(size = 14)       # Increase legend title size
  )

# Diagrama de violín para SpO2
ggplot(data_long, aes(x = Device, y = SpO2, fill = Device)) +
  geom_violin(trim = FALSE) +
  labs(title = "Comparación de valores de SpO2 en los dispositivos",
       x = "Dispositivo",
       y = "SpO2 (%)",
       fill = "Dispositivo") +
  theme_minimal() +
  theme(
    plot.title = element_text(size = 18),        # Increase title size
    axis.title.x = element_text(size = 14),      # Increase x-axis title size
    axis.title.y = element_text(size = 14),      # Increase y-axis title size
    axis.text.x = element_text(size = 11),       # Increase x-axis text (tick labels) size
    axis.text.y = element_text(size = 11),       # Increase y-axis text (tick labels) size
    legend.title = element_text(size = 14)       # Increase legend title size
  )

# Diagrama de violín para LPM
ggplot(data_long, aes(x = Device, y = BPM, fill = Device)) +
  geom_violin(trim = FALSE) +
  labs(title = "Comparación de valores de latidos por minuto en los dispositivos",
       x = "Dispositivo",
       y = "Latidos por minuto (LPM)",
       fill = "Dispositivo") +  # Change the legend title here
  theme_minimal() +
  theme(
    plot.title = element_text(size = 18),        # Increase title size
    axis.title.x = element_text(size = 14),      # Increase x-axis title size
    axis.title.y = element_text(size = 14),      # Increase y-axis title size
    axis.text.x = element_text(size = 11),       # Increase x-axis text (tick labels) size
    axis.text.y = element_text(size = 11),       # Increase y-axis text (tick labels) size
    legend.title = element_text(size = 14)       # Increase legend title size
  )


# ANOVA para SpO2
anova_spo2 <- aov(SpO2 ~ Device, data = data_long)
summary(anova_spo2)

# ANOVA para BPM
anova_bpm <- aov(BPM ~ Device, data = data_long)
summary(anova_bpm)

# Post-hoc test: (Tukey's HSD) para SpO2
tukey_spo2 <- TukeyHSD(anova_spo2)
print(tukey_spo2)

# Post-hoc test: (Tukey's HSD) para LPM
tukey_bpm <- TukeyHSD(anova_bpm)
print(tukey_bpm)

# Creación de boxplots individuales para cada persona (SpO2 y LPM)

# Boxplot para SpO2 para cada persona
ggplot(data_long, aes(x = Device, y = SpO2, fill = Device)) +
  geom_boxplot() +
  stat_summary(fun = mean, geom = "point", shape = 21, size = 3, fill = "yellow", color = "black") +  # Add mean points
  labs(title = "Comparación de valores de SpO2 en los dispositivos", 
       x = "Dispositivo", 
       y = "SpO2 (%)",
       fill = "Dispositivo") +
  theme_minimal() +
  theme(
    plot.title = element_text(size = 18),        # Increase title size
    axis.title.x = element_text(size = 14),      # Increase x-axis title size
    axis.title.y = element_text(size = 14),      # Increase y-axis title size
    axis.text.x = element_text(size = 11),       # Increase x-axis text (tick labels) size
    axis.text.y = element_text(size = 11),       # Increase y-axis text (tick labels) size
    legend.title = element_text(size = 14)       # Increase legend title size
  ) +
  facet_wrap(~ Person, ncol = 4, labeller = labeller(Person = function(x) paste("Persona", x)))  # Create a facet for each person with 4 columns
  
# Boxplot para LPM para cada persona
ggplot(data_long, aes(x = Device, y = BPM, fill = Device)) +
  geom_boxplot() +
  stat_summary(fun = mean, geom = "point", shape = 21, size = 3, fill = "yellow", color = "black") +  # Add mean points
  labs(title = "Comparación de valores de latidos por minuto en los dispositivos", 
       x = "Dispositivo", 
       y = "Latidos por minuto (LPM)",
       fill = "Dispositivo") +
  theme_minimal() +
  theme(
    plot.title = element_text(size = 18),        # Increase title size
    axis.title.x = element_text(size = 14),      # Increase x-axis title size
    axis.title.y = element_text(size = 14),      # Increase y-axis title size
    axis.text.x = element_text(size = 11),       # Increase x-axis text (tick labels) size
    axis.text.y = element_text(size = 11),       # Increase y-axis text (tick labels) size
    legend.title = element_text(size = 14)       # Increase legend title size
  ) +
  facet_wrap(~ Person, ncol = 4, labeller = labeller(Person = function(x) paste("Persona", x)))  # Create a facet for each person with 4 columns