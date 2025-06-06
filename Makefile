NAME = webserv

# Dossiers
OBJ_DIR = objs
SRC_DIR = srcs
INC_DIR = includes
DEP_DIR = deps

# Création des dossiers nécessaires
$(shell mkdir -p $(OBJ_DIR) $(DEP_DIR))

CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98 -g3

MAIN = webserv

SRC = ${addsuffix .cpp, ${MAIN}} \
      server/Server.cpp \
      parsing/Config.cpp \
	  utils/Utils.cpp \
	  parsing/Locations.cpp \
	  server/Request.cpp \
	  server/Response.cpp \
	  methods/AMethods.cpp \
	  methods/Get.cpp \
	  methods/Post.cpp \
	  methods/Delete.cpp \
	  methods/CGIhandler.cpp \
	  server/EpollManager.cpp \

SRC_DIR = srcs

SRCS = $(addprefix $(SRC_DIR)/, $(SRC))

# Transformation des chemins pour mettre tous les objets dans un seul dossier
OBJ = $(SRC:.cpp=.o)
OBJ := $(subst /,_,$(OBJ))
OBJS = $(addprefix $(OBJ_DIR)/, $(OBJ))

# Fichiers de dépendances
DEPS = $(SRC:.cpp=.d)
DEPS := $(subst /,_,$(DEPS))
DEPS := $(addprefix $(DEP_DIR)/, $(DEPS))

# Couleurs et styles
GREEN = \033[0;32m
BLUE = \033[0;34m
RED = \033[0;31m
RESET = \033[0m
BOLD = \033[1m

# Compteurs pour le pourcentage
TOTAL_FILES := $(words $(SRCS))
CURRENT_FILE = 0

all: create_dirs $(NAME)
				 $(print_flag)

# Création des dossiers nécessaires (maintenant une cible plutôt qu'une directive shell)
create_dirs:
	@mkdir -p $(OBJ_DIR) $(DEP_DIR)

$(NAME): $(OBJS)
	@echo "$(GREEN)$(BOLD)Linking $(NAME)...$(RESET)"
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)$(BOLD)✓ $(NAME) compilé avec succès!$(RESET)"

$(OBJ_DIR)/%_Server.o: $(SRC_DIR)/server/Server.cpp
	@$(eval CURRENT_FILE := $(shell echo $$(($(CURRENT_FILE) + 1))))
	@printf "$(BLUE)$(BOLD)Compilation [%d%%] %s$(RESET)\r" $$(( $(CURRENT_FILE) * 100 / $(TOTAL_FILES) )) $<
	@$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*_Server.d -c $< -o $@ -I$(INC_DIR)
	@if [ $(CURRENT_FILE) = $(TOTAL_FILES) ]; then echo ""; fi

$(OBJ_DIR)/%_Config.o: $(SRC_DIR)/parsing/Config.cpp
	@$(eval CURRENT_FILE := $(shell echo $$(($(CURRENT_FILE) + 1))))
	@printf "$(BLUE)$(BOLD)Compilation [%d%%] %s$(RESET)\r" $$(( $(CURRENT_FILE) * 100 / $(TOTAL_FILES) )) $<
	@$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*_Config.d -c $< -o $@ -I$(INC_DIR)
	@if [ $(CURRENT_FILE) = $(TOTAL_FILES) ]; then echo ""; fi

$(OBJ_DIR)/%_Utils.o: $(SRC_DIR)/utils/Utils.cpp
	@$(eval CURRENT_FILE := $(shell echo $$(($(CURRENT_FILE) + 1))))
	@printf "$(BLUE)$(BOLD)Compilation [%d%%] %s$(RESET)\r" $$(( $(CURRENT_FILE) * 100 / $(TOTAL_FILES) )) $<
	@$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*_Utils.d -c $< -o $@ -I$(INC_DIR)
	@if [ $(CURRENT_FILE) = $(TOTAL_FILES) ]; then echo ""; fi

$(OBJ_DIR)/%_Locations.o: $(SRC_DIR)/parsing/Locations.cpp
	@$(eval CURRENT_FILE := $(shell echo $$(($(CURRENT_FILE) + 1))))
	@printf "$(BLUE)$(BOLD)Compilation [%d%%] %s$(RESET)\r" $$(( $(CURRENT_FILE) * 100 / $(TOTAL_FILES) )) $<
	@$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*_Locations.d -c $< -o $@ -I$(INC_DIR)
	@if [ $(CURRENT_FILE) = $(TOTAL_FILES) ]; then echo ""; fi

$(OBJ_DIR)/webserv.o: $(SRC_DIR)/webserv.cpp
	@$(eval CURRENT_FILE := $(shell echo $$(($(CURRENT_FILE) + 1))))
	@printf "$(BLUE)$(BOLD)Compilation [%d%%] %s$(RESET)\r" $$(( $(CURRENT_FILE) * 100 / $(TOTAL_FILES) )) $<
	@$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/webserv.d -c $< -o $@ -I$(INC_DIR)
	@if [ $(CURRENT_FILE) = $(TOTAL_FILES) ]; then echo ""; fi

$(OBJ_DIR)/%_Request.o: $(SRC_DIR)/server/Request.cpp
	@$(eval CURRENT_FILE := $(shell echo $$(($(CURRENT_FILE) + 1))))
	@printf "$(BLUE)$(BOLD)Compilation [%d%%] %s$(RESET)\r" $$(( $(CURRENT_FILE) * 100 / $(TOTAL_FILES) )) $<
	@$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*_Request.d -c $< -o $@ -I$(INC_DIR)
	@if [ $(CURRENT_FILE) = $(TOTAL_FILES) ]; then echo ""; fi

$(OBJ_DIR)/%_Response.o: $(SRC_DIR)/server/Response.cpp
	@$(eval CURRENT_FILE := $(shell echo $$(($(CURRENT_FILE) + 1))))
	@printf "$(BLUE)$(BOLD)Compilation [%d%%] %s$(RESET)\r" $$(( $(CURRENT_FILE) * 100 / $(TOTAL_FILES) )) $<
	@$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*_Response.d -c $< -o $@ -I$(INC_DIR)
	@if [ $(CURRENT_FILE) = $(TOTAL_FILES) ]; then echo ""; fi

$(OBJ_DIR)/%_AMethods.o: $(SRC_DIR)/methods/AMethods.cpp
	@$(eval CURRENT_FILE := $(shell echo $$(($(CURRENT_FILE) + 1))))
	@printf "$(BLUE)$(BOLD)Compilation [%d%%] %s$(RESET)\r" $$(( $(CURRENT_FILE) * 100 / $(TOTAL_FILES) )) $<
	@$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*_AMethods.d -c $< -o $@ -I$(INC_DIR)
	@if [ $(CURRENT_FILE) = $(TOTAL_FILES) ]; then echo ""; fi

$(OBJ_DIR)/%_Get.o: $(SRC_DIR)/methods/Get.cpp
	@$(eval CURRENT_FILE := $(shell echo $$(($(CURRENT_FILE) + 1))))
	@printf "$(BLUE)$(BOLD)Compilation [%d%%] %s$(RESET)\r" $$(( $(CURRENT_FILE) * 100 / $(TOTAL_FILES) )) $<
	@$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*_Get.d -c $< -o $@ -I$(INC_DIR)
	@if [ $(CURRENT_FILE) = $(TOTAL_FILES) ]; then echo ""; fi

$(OBJ_DIR)/%_Post.o: $(SRC_DIR)/methods/Post.cpp
	@$(eval CURRENT_FILE := $(shell echo $$(($(CURRENT_FILE) + 1))))
	@printf "$(BLUE)$(BOLD)Compilation [%d%%] %s$(RESET)\r" $$(( $(CURRENT_FILE) * 100 / $(TOTAL_FILES) )) $<
	@$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*_Post.d -c $< -o $@ -I$(INC_DIR)
	@if [ $(CURRENT_FILE) = $(TOTAL_FILES) ]; then echo ""; fi

$(OBJ_DIR)/%_Delete.o: $(SRC_DIR)/methods/Delete.cpp
	@$(eval CURRENT_FILE := $(shell echo $$(($(CURRENT_FILE) + 1))))
	@printf "$(BLUE)$(BOLD)Compilation [%d%%] %s$(RESET)\r" $$(( $(CURRENT_FILE) * 100 / $(TOTAL_FILES) )) $<
	@$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*_Delete.d -c $< -o $@ -I$(INC_DIR)
	@if [ $(CURRENT_FILE) = $(TOTAL_FILES) ]; then echo ""; fi

$(OBJ_DIR)/%_CGIhandler.o: $(SRC_DIR)/methods/CGIhandler.cpp
	@$(eval CURRENT_FILE := $(shell echo $$(($(CURRENT_FILE) + 1))))
	@printf "$(BLUE)$(BOLD)Compilation [%d%%] %s$(RESET)\r" $$(( $(CURRENT_FILE) * 100 / $(TOTAL_FILES) )) $<
	@$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*_CGIhandler.d -c $< -o $@ -I$(INC_DIR)
	@if [ $(CURRENT_FILE) = $(TOTAL_FILES) ]; then echo ""; fi


$(OBJ_DIR)/%_EpollManager.o: $(SRC_DIR)/server/EpollManager.cpp
	@$(eval CURRENT_FILE := $(shell echo $$(($(CURRENT_FILE) + 1))))
	@printf "$(BLUE)$(BOLD)Compilation [%d%%] %s$(RESET)\r" $$(( $(CURRENT_FILE) * 100 / $(TOTAL_FILES) )) $<
	@$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*_EpollManager.d -c $< -o $@ -I$(INC_DIR)
	@if [ $(CURRENT_FILE) = $(TOTAL_FILES) ]; then echo ""; fi


clean:
	@echo "$(RED)$(BOLD)Nettoyage des fichiers objets...$(RESET)"
	@rm -rf $(OBJ_DIR)
	@rm -rf $(DEP_DIR)
	@echo "$(GREEN)$(BOLD)✓ Nettoyage terminé!$(RESET)"

fclean: clean
	@echo "$(RED)$(BOLD)Suppression de $(NAME)...$(RESET)"
	@rm -f $(NAME)
	@echo "$(GREEN)$(BOLD)✓ Suppression terminée!$(RESET)"

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re


# **************************************************************************** #
#                              COLOR SETTING                                   #
# **************************************************************************** #

COLOR_RESET = \033[0m
PINKB = \033[1;95m
REDB = \033[1;91m
ROSEB = \033[1;38;5;225m
BLUEB = \033[1;34m
BABEBLUEB = \033[1;38;5;153m
GREENB = \033[1;38;5;85m
PURPLEB = \033[1;38;5;55m
PSTL_YELLOWB = \033[1;38;2;255;253;208m
PSTL_ORGB = \033[1;38;2;255;179;102m
PSTL_PURPLEB =\033[1;38;2;204;153;255m

GREEN_BBG = \033[1;42m
BLUE_BBG = \033[1;44m
YELLOW_BBG = \033[1;43m
ORANGE_BBG = \033[1;48;5;214m

# **************************************************************************** #
#                               PRINT FLAG                                     #
# **************************************************************************** #

define print_flag
	@echo ""
	@echo "$(GREENB)     𓂃.☘︎ ݁˖.𖥔 ݁ ˖𓂃.☘︎ ݁˖.𖥔 ݁ ˖𓂃.☘︎ ݁˖.𖥔 ݁ ˖𓂃.☘︎ ݁˖.𖥔 ݁ ˖𓂃.☘︎ ݁˖˖𓂃.☘︎ ݁˖˖𓂃.☘︎݁˖˖𓂃.☘︎𓂃.☘︎𓂃.☘︎$(COLOR_RESET)"
	@echo "$(BABEBLUEB)"
	@cat getlucky_banner.txt
	@echo "$(COLOR_RESET)"
	@echo "$(GREENB)     𓂃.☘︎ ݁˖.𖥔 ݁ ˖𓂃.☘︎ ݁˖.𖥔 ݁ ˖𓂃.☘︎ ݁˖.𖥔 ݁ ˖𓂃.☘︎ ݁˖.𖥔 ݁ ˖𓂃.☘︎ ݁˖˖𓂃.☘︎ ݁˖˖𓂃.☘︎݁˖˖𓂃.☘︎𓂃.☘︎𓂃.☘︎$(COLOR_RESET)"
endef

# @echo "$(PURPLEB)"
# @cat getlucky_banner2.txt
# @echo "$(COLOR_RESET)"
