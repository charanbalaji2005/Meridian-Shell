import React from 'react';
import { Home, ChevronRight } from 'lucide-react';

interface BreadcrumbsProps {
  category: string;
  title: string;
  onNavigateHome: () => void;
}

export const Breadcrumbs: React.FC<BreadcrumbsProps> = ({
  category,
  title,
  onNavigateHome,
}) => {
  return (
    <nav className="fedora-breadcrumbs" aria-label="Breadcrumbs">
      <button onClick={onNavigateHome} className="breadcrumb-home" title="Documentation Home">
        <Home size={14} />
      </button>
      <ChevronRight size={12} className="breadcrumb-separator" />
      <span className="breadcrumb-item breadcrumb-cat">{category}</span>
      <ChevronRight size={12} className="breadcrumb-separator" />
      <span className="breadcrumb-item breadcrumb-current">{title}</span>
    </nav>
  );
};
